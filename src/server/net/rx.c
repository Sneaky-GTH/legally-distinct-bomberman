#include "server/net/rx.h"
#include "server/net/args.h"
#include "lib/protocol/messages.h"
#include "lib/net.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define CLIENT_BUF_CAP 4096
#define MAX_CLIENTS 16

typedef struct {
    int     fd;
    uint8_t buf[CLIENT_BUF_CAP];
    int     buf_len;
} ClientState;

void* rx_thread(void* arg) {

    RxArgs* args = (RxArgs*)arg;
    ClientState clients[MAX_CLIENTS] = {0};
    struct epoll_event events[MAX_CLIENTS];

    while (1) {

        int n = epoll_wait(args->sh.epfd, events, MAX_CLIENTS, 20);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            // HANDLE NEW CLIENT
            if (fd == args->sh.server_fd) {

                int client_fd = accept(args->sh.server_fd, NULL, NULL);
                if (client_fd < 0) continue;

                fcntl(client_fd, F_SETFL, O_NONBLOCK);

                // find a free slot
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j].fd == 0) {
                        clients[j].fd = client_fd;
                        clients[j].buf_len = 0;
                        break;
                    }
                }

                // register with epoll
                struct epoll_event cev = {
                    .events  = EPOLLIN | EPOLLRDHUP,
                    .data.fd = client_fd
                };
                epoll_ctl(args->sh.epfd, EPOLL_CTL_ADD, client_fd, &cev);


            // HANDLE CLIENT DISCONNECTING
            } else if (events[i].events & EPOLLRDHUP) {
                epoll_ctl(args->sh.epfd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);

                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j].fd == fd) {
                        clients[j].fd = 0;
                        clients[j].buf_len = 0;
                        break;
                    }
                }

            // HANDLE DATA FROM EXISTING CLIENT
            } else if (events[i].events & EPOLLIN) {

                // find this client's state
                ClientState *client = NULL;
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j].fd == fd) {
                        client = &clients[j];
                        break;
                    }
                }
                if (!client) continue;

                // try to parse one or more complete messages from this client's buffer
                Message msg;
                int res = recv_message(
                    client->fd,
                    client->buf,
                    &client->buf_len,
                    CLIENT_BUF_CAP,
                    &msg
                );

                if (res == ERECVCLOSED) {
                    // clean disconnect
                    epoll_ctl(args->sh.epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    client->fd = 0;
                    client->buf_len = 0;

                } else if (res > 0) {
                    // got a complete message — push to game thread


                    ClientMessage cmsg = {
                        .msg = msg,
                        .fd = client->fd
                    };

                    pthread_mutex_lock(&args->input->lock);
                    args->input->messages[args->input->tail] = cmsg;
                    args->input->tail = (args->input->tail + 1) % MAX_QUEUE;
                    args->input->count++;
                    pthread_cond_signal(&args->input->not_empty);
                    pthread_mutex_unlock(&args->input->lock);
                }

            } // end of if block

        } // end of for cycle

    } // end of while statement

    return NULL;
}
