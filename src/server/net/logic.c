#include "server/net/logic.h"
#include "server/net/args.h"
#include <stdio.h>
#include <protocol/messages.h>
#include <pthread.h>

void process_action(ClientMessage* msg, MessageQueue* output) {
    printf("Processing a message!\n");
    printf("This message has the type: %d\n", msg->msg.type);

    struct Message send_msg = {
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

    ClientMessage newmsg = {
        .msg = send_msg,
        .fd = msg->fd
    };

    pthread_mutex_lock(&output->lock);
    output->messages[output->tail] = newmsg;
    output->tail = (output->tail + 1) % MAX_QUEUE;
    output->count++;
    pthread_cond_signal(&output->not_empty);
    pthread_mutex_unlock(&output->lock);

    printf("Message added to TX queue, good luck!\n");

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
