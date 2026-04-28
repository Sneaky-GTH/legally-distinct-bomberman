#include "server/net/game.h"
#include "args.h"
#include "game.h"
#include "server/net/args.h"
#include "server/logic/messagehandling.h"
#include <stdio.h>
#include <protocol/messages.h>
#include <pthread.h>

Client clients[MAX_CLIENTS];

void send_to_client(int c, Message msg, MessageQueue* output) {

    ClientMessage cmsg = {
        .fd = c,
        .msg = msg
    };

    pthread_mutex_lock(&output->lock);
    output->messages[output->tail] = cmsg;
    output->tail = (output->tail + 1) % MAX_QUEUE;
    output->count++;
    pthread_cond_signal(&output->not_empty);
    pthread_mutex_unlock(&output->lock);


    printf("GAME INFO: Message added to TX queue, good luck TX!\n");
}


void broadcast_to_clients(Client* clients, Message msg, MessageQueue* output) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        send_to_client(clients[i].fd, msg, output);
    }
}


void process_action(ClientMessage* msg, MessageQueue* output) {
    printf("GAME INFO: Game thread is processing a message!\n");
    printf("This message has the type: %d\n", msg->msg.type);

    Message send_msg = {
        .type = MSG_WELCOME,
        .sender_id = 255, // Will be assigned by server
        .target_id = 0, // Server
        .data.welcome = {
            .server_name = "bomboclat-express",
            .status = GAME_LOBBY,
            .len = 0,
            .clients = NULL
        },
    };

    switch(msg->msg.type) {
        case MSG_HELLO:
            send_to_client(msg->fd, srv_process_hello(NULL, &msg->msg), output);
            break;
        default:
            printf("GAME ERR: Game server received unknown message type.\n");
            break;
    }

}

void *game_thread(void* arg) {
    GameArgs* args = (GameArgs*)arg;
    while (1) {

        struct timespec deadline;
        clock_gettime(CLOCK_MONOTONIC, &deadline);
        deadline.tv_nsec += 20 * 1000000;  // 20ms from now
        if (deadline.tv_nsec >= 1000000000) {
            deadline.tv_sec  += 1;
            deadline.tv_nsec -= 1000000000;
        }

        pthread_mutex_lock(&args->input->lock);

        // wait for queue to be nonempty so theres something to process
        while (args->input->count == 0) {
            pthread_cond_timedwait(&args->input->not_empty, &args->input->lock, &deadline);
        }

        ClientMessage msg = args->input->messages[args->input->head];
        args->input->head = (args->input->head + 1) % MAX_QUEUE;
        args->input->count--;

        pthread_mutex_unlock(&args->input->lock);

        process_action(&msg, args->output);

    }
    return NULL;
}
