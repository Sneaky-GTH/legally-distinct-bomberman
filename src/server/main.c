#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <protocol/messages.h>
#include "server/logic/playingfield.h"
#include "server/logic/player.h"
#include "server/net/args.h"
#include "server/net/game.h"
#include "server/net/rx.h"
#include "server/net/tx.h"

#define MSG_SIZE 4096

ServerHandle setup_epoll(int port) {

    // create the socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // allow reusing the socket for easier testing
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // set up the sockaddr
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY   // accept connections on any interface
    };

    // bind it to the socket
    int s = bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (s != 0) return (ServerHandle){ .epfd = -1, .server_fd = -1 };

    listen(server_fd, 16);

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    // create the epoll instance and add it to the epoll controller
    int epfd = epoll_create1(0);

    struct epoll_event ev = {
        .events  = EPOLLIN,     // notify when a new connection arrives
        .data.fd = server_fd
    };

    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    ServerHandle sh = {
        .epfd = epfd,
        .server_fd = server_fd
    };

    // return :)
    return sh;
}


int main(void) {

    MessageQueue input_queue  = {
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .not_empty = PTHREAD_COND_INITIALIZER
    };

    MessageQueue output_queue = {
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .not_empty = PTHREAD_COND_INITIALIZER
    };

    ServerHandle sh = setup_epoll(7500);

    // each thread gets only what it needs
    RxArgs rx_args = { .input  = &input_queue, .sh = sh };
    GameArgs game_args = { .input = &input_queue, .output = &output_queue };
    TxArgs tx_args = { .output = &output_queue};

    pthread_t rx_tid, game_tid, tx_tid;
    pthread_create(&rx_tid,   NULL, rx_thread,   &rx_args);
    pthread_create(&game_tid, NULL, game_thread, &game_args);
    pthread_create(&tx_tid,   NULL, tx_thread,   &tx_args);

    pthread_join(rx_tid,   NULL);
    pthread_join(game_tid, NULL);
    pthread_join(tx_tid,   NULL);

    return 0;

}
