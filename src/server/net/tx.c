#include "server/net/tx.h"
#include "server/net/args.h"
#include "lib/protocol/messages.h"
#include "lib/net.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>

#define CLIENT_BUF_CAP 4096


void *tx_thread(void *arg) {
    TxArgs* args = (TxArgs*)arg;

    while (1) {
        // blocks waiting for game thread to push something
        pthread_cond_wait(&args->output->not_empty, &args->output->lock);


        ClientMessage msg = args->output->messages[args->output->head];
        args->output->head = (args->output->head + 1) % MAX_QUEUE;
        args->output->count--;

        printf("TX INFO: TX thread has received a message, trying to send it...\n");

        // send() might block here for a slow client
        // but that's fine — only THIS thread stalls, not the game tick
        int s = send_message(msg.fd, &msg.msg);

        if (s == 0) {
            printf("TX INFO: TX thread sent message - good luck on your journey!\n");
        } else {
            printf("TX ERR: Something when wrong when the TX thread tried to send a message.\n");
        }

    }

}
