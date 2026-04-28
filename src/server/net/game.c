#include "server/net/game.h"
#include "server/net/args.h"
#include "server/logic/messagehandling.h"
#include <stdio.h>
#include <protocol/messages.h>
#include <pthread.h>

GameState gamestate;

int add_new_client(Client clients[MAX_CLIENTS], int fd) {

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].p.id == 0) {
            clients[i].fd = fd;
            clients[i].p.id = i;
            return i;
        }
    }

    return -1;

}

int remove_client(Client clients[MAX_CLIENTS], int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].p.id == i) {
            clients[i].fd = 0;
            reset_player(&clients[i].p, i, 0, 0);
            return i;
        }
    }

    return -1;
}

void send_to_client(int fd, Message msg, MessageQueue* output) {

    ClientMessage cmsg = {
        .fd = fd,
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


void broadcast_to_clients(Client clients[MAX_CLIENTS], Message msg, MessageQueue* output) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].p.id == 0) continue;
        send_to_client(clients[i].fd, msg, output);
    }
}


void process_action(ClientMessage* rx_msg, MessageQueue* output) {
    printf("GAME INFO: Game thread is processing a message!\n");
    printf("This message has the type: %d\n", rx_msg->msg.type);

    Message tx_msg;
    uint8_t res;

    switch(rx_msg->msg.type) {

        // --------------- MSG_HELLO ---------------
        case MSG_HELLO:
            srv_process_hello(&gamestate, &rx_msg->msg);

            tx_msg = (Message){
                .type = MSG_WELCOME,
                .sender_id = 255,
                .target_id = add_new_client(gamestate.clients, rx_msg->fd),
                .data.welcome = {
                    .server_name = "bomboclat-express",
                    .status = GAME_LOBBY,
                    .len = 0,
                    .clients = NULL
                },
            };

            send_to_client(rx_msg->fd, tx_msg, output);
            break;
        case MSG_LEAVE:
            rx_msg->msg.target_id = remove_client(gamestate.clients, rx_msg->fd);
            broadcast_to_clients(gamestate.clients, rx_msg->msg, output);
            break;

        // --------------- MSG_MOVE_ATTEMPT ---------------
        case MSG_MOVE_ATTEMPT:
            res = srv_process_move_attempt(&gamestate, &rx_msg->msg);
            if (res != 0) return;

            tx_msg = (Message){
                .type = MSG_MOVED,
                .sender_id = 255,
                .target_id = 254,
                .data.moved = {
                    .player_id = rx_msg->msg.sender_id,
                    .new_position = res
                },
            };

            broadcast_to_clients(gamestate.clients, tx_msg, output);
            break;

        // --------------- MSG_PING ---------------
        case MSG_PING:
            tx_msg = (Message){
                .type = MSG_PONG,
                .sender_id = 255,
                .target_id = rx_msg->msg.sender_id,
            };

            send_to_client(rx_msg->fd, tx_msg, output);
            printf("GAME INFO: POOOOONNNNNGGGGG!!!!\n");
            break;

        // --------------- MSG_BOMB_ATTEMPT ---------------
        case MSG_BOMB_ATTEMPT:
            res = srv_process_bomb_attempt(&gamestate, &rx_msg->msg);
            if (res != 0) return;

            tx_msg = (Message){
                .type = MSG_BOMB,
                .sender_id = 255,
                .target_id = 254,
                .data.moved = {
                    .player_id = rx_msg->msg.sender_id,
                    .new_position = res
                },
            };

            broadcast_to_clients(gamestate.clients, tx_msg, output);
            break;

        // --------------- MSG_EPIC_FAIL ---------------
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
