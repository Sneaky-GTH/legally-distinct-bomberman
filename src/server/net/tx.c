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
#define MAX_CLIENTS 16


void *tx_thread(void *arg) {
    TxArgs* args = (TxArgs*)arg;

    while (1) {
        // blocks waiting for game thread to push something
        pthread_cond_wait(&args->output->not_empty, &args->output->lock);


        ClientMessage msg = args->output->messages[args->output->head];
        args->output->head = (args->output->head + 1) % MAX_QUEUE;
        args->output->count--;

        printf("Message received, trying to send it...\n");

        // send() might block here for a slow client
        // but that's fine — only THIS thread stalls, not the game tick
        send_message(msg.fd, &msg.msg);
    }
}
