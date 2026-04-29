#include "server/net/game.h"
#include "args.h"
#include "game.h"
#include "server/net/args.h"
#include "server/logic/messagehandling.h"
#include <stdio.h>
#include <protocol/messages.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

GameState gamestate;

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
        printf("GAME INFO: Sending message to TX queue for client: %d\n", clients[i].p.id);
    }
}


void print_clients(GameState* game) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id == 0) continue;
        printf("GAME DEBUG: Client: %d\n", game->clients[i].p.id);
    }
}


void check_player_death(GameState* game, ServerMessage* servermessages) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id == 0) continue;
        if (SAFE_GET_CELL(&game->wallmap, game->clients[i].p.x, game->clients[i].p.y) == 'X') {
            game->clients[i].is_alive = 0;

            while (servermessages->nextmsg != NULL) {
                servermessages = servermessages->nextmsg;
            }

            ServerMessage* next = malloc(sizeof(ServerMessage));
            next->nextmsg = NULL;
            servermessages->nextmsg = (struct ServerMessage*)next;

            Message tx_msg = (Message){
                .type = MSG_DEATH,
                .sender_id = 255,
                .target_id = 254,
                .data.death= {
                    .player_id = game->clients[i].p.id,
                }
            };

            servermessages->has_content = 1;
            servermessages->msg = tx_msg;

        }
    }
}


void spread_out_players(GameState* game, MessageQueue* output) {

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id == 0) continue;

        uint8_t res;

        switch(game->clients[i].p.id) {
            case 1:
                res = player_set_spawn(&game->playermap, &game->clients[i].p, 0, 0, '1');
                break;
            case 2:
                res = player_set_spawn(&game->playermap, &game->clients[i].p, game->playermap.width - 1, 0, '1');
                break;
            case 3:
                res = player_set_spawn(&game->playermap, &game->clients[i].p, 0, game->playermap.height - 1, '1');
                break;
            case 4:
                res = player_set_spawn(&game->playermap, &game->clients[i].p, game->playermap.width - 1, game->playermap.height - 1, '1');
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            case 8:
                break;
            default:
                break;
        }

        Message tx_msg = (Message){
            .type = MSG_MOVED,
            .sender_id = 255,
            .target_id = 254,
            .data.moved = {
                .player_id = game->clients[i].p.id,
                .new_position = res
            },
        };

        printf("GAME INFO: Sending move with position: %d\n", res);

        broadcast_to_clients(gamestate.clients, tx_msg, output);

    }

}


void process_action(ClientMessage* rx_msg, MessageQueue* output) {
    printf("GAME INFO: Game thread is processing a message!\n");
    printf("GAME INFO: This message has the type: %d\n", rx_msg->msg.type);

    Message tx_msg;
    uint8_t res;

    print_clients(&gamestate);

    switch(rx_msg->msg.type) {

        // --------------- MSG_HELLO ---------------
        case MSG_HELLO:

            int new_id = srv_process_hello(&gamestate, rx_msg);

            clients_t client_buf[MAX_CLIENTS];

            for (int i = 0; i < gamestate.client_count; i++) {
                client_buf[i].client_id = gamestate.clients[i].p.id;
                client_buf[i].is_ready = gamestate.clients[i].is_ready;
                strncpy(client_buf[i].client_name, gamestate.clients[i].name, 30);
            }

            tx_msg = (Message){
                .type = MSG_WELCOME,
                .sender_id = 255,
                .target_id = new_id,
                .data.welcome = {
                    .status = gamestate.status,
                    .len = gamestate.client_count,
                    .clients = client_buf,
                }
            };
            strncpy(tx_msg.data.welcome.server_name, "bomboclat-express", 20);

            send_to_client(rx_msg->fd, tx_msg, output);

            tx_msg = (Message){
                .type = MSG_HELLO,
                .sender_id = new_id,
                .target_id = 254,
            };

            strncpy(tx_msg.data.hello.client_id, rx_msg->msg.data.hello.client_id, 20);
            strncpy(tx_msg.data.hello.client_name, rx_msg->msg.data.hello.client_name, 30);

            broadcast_to_clients(gamestate.clients, tx_msg, output);
            break;

        // --------------- MSG_LEAVE ---------------
        case MSG_LEAVE:
            rx_msg->msg.target_id = remove_client(gamestate.clients, rx_msg->fd);
            broadcast_to_clients(gamestate.clients, rx_msg->msg, output);
            break;

        // --------------- MSG_MOVE_ATTEMPT ---------------
        case MSG_MOVE_ATTEMPT:
            res = srv_process_move_attempt(&gamestate, &rx_msg->msg);
            if (res < 0) return;

            tx_msg = (Message){
                .type = MSG_MOVED,
                .sender_id = 255,
                .target_id = 254,
                .data.moved = {
                    .player_id = rx_msg->msg.sender_id,
                    .new_position = (uint8_t)res
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

            if (res < 0) return;

            tx_msg = (Message){
                .type = MSG_BOMB,
                .sender_id = 255,
                .target_id = 254,
                .data.moved = {
                    .player_id = rx_msg->msg.sender_id,
                    .new_position = (uint8_t)res
                },
            };

            broadcast_to_clients(gamestate.clients, tx_msg, output);

            break;

        // --------------- MSG_READY ---------------
        case MSG_SET_READY:
        {
            res = srv_process_ready(&gamestate, &rx_msg->msg);

            broadcast_to_clients(gamestate.clients, tx_msg, output);

            if (res != 0) return;

            cell_types_t map_buf[gamestate.wallmap.width * gamestate.wallmap.height];

            for (int i = 0; i < gamestate.wallmap.width * gamestate.wallmap.height; i++) {
                map_buf[i] = gamestate.wallmap.cell[i];
            }

            tx_msg = (Message){
                .type = MSG_MAP,
                .sender_id = 255,
                .target_id = 254,
                .data.map = {
                    .height = gamestate.wallmap.height,
                    .width = gamestate.wallmap.width,
                    .map = map_buf,
                }
            };

            broadcast_to_clients(gamestate.clients, tx_msg, output);

            spread_out_players(&gamestate, output);

            tx_msg = (Message){
                .type = MSG_SET_STATUS,
                .sender_id = 255,
                .target_id = 254,
                .data.set_status = {
                    .status = 1,
                },
            };

            gamestate.status = 1;

            broadcast_to_clients(gamestate.clients, tx_msg, output);

            break;
        }

        // --------------- MSG_EPIC_FAIL ---------------
        default:
            printf("GAME ERR: Game server received unknown message type.\n");
            break;
    }

}


void setup_game(GameState* game) {

    for (int i = 0; i < MAX_CLIENTS; i++) {
        game->clients[i].fd = 0;
        game->clients[i].p.id = 0;
        game->clients[i].is_ready = 0;
        game->clients[i].is_alive = 1;
        reset_player(&game->clients[i].p, 0, 255, 255);
    }

    game->bombs = NULL;
    game->explosions = NULL;
    game->status = 0;
    game->client_count = 0;

    init_playingField(&game->playermap, 10, 10);
    init_playingField(&game->wallmap, 10, 10);
}

void gametick(GameState* game, MessageQueue* output) {

    if(game->status != 1) return;

    ServerMessage* servermessages = malloc(sizeof(ServerMessage));
    servermessages->nextmsg = NULL;
    servermessages->has_content = 0;

    //printf("GAME INFO: Processing tick...\n");

    process_bombs(game, servermessages);
    process_antibombs(game, servermessages);
    process_explosions(game);
    check_player_death(game, servermessages);

    while(servermessages->has_content == 1) {
        ServerMessage* next = servermessages->nextmsg;

        broadcast_to_clients(game->clients, servermessages->msg, output);

        free(servermessages);
        servermessages = next;
        if (servermessages == NULL) break;
    }
}

static int timespec_passed(const struct timespec *t) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec > t->tv_sec ||
    (now.tv_sec == t->tv_sec && now.tv_nsec >= t->tv_nsec);
}

void *game_thread(void* arg) {
    GameArgs* args = (GameArgs*)arg;
    setup_game(&gamestate);

    struct timespec next_tick;
    clock_gettime(CLOCK_MONOTONIC, &next_tick);

    while (1) {
        // build deadline 16ms from last tick (not from now)
        next_tick.tv_nsec += 16L * 1000000L;
        if (next_tick.tv_nsec >= 1000000000L) {
            next_tick.tv_sec  += 1;
            next_tick.tv_nsec -= 1000000000L;
        }


        pthread_mutex_lock(&args->input->lock);

        // wait for at least one message, or until tick is due
        while (args->input->count == 0) {
            int rc = pthread_cond_timedwait(&args->input->not_empty,
                                            &args->input->lock, &next_tick);
            if (rc == ETIMEDOUT) break;
        }

        // drain messages until queue empty or tick deadline passed
        while (args->input->count > 0 && !timespec_passed(&next_tick)) {
            ClientMessage msg = args->input->messages[args->input->head];
            args->input->head = (args->input->head + 1) % MAX_QUEUE;
            args->input->count--;
            pthread_mutex_unlock(&args->input->lock);

            process_action(&msg, args->output);

            pthread_mutex_lock(&args->input->lock);
        }

        pthread_mutex_unlock(&args->input->lock);

        gametick(&gamestate, args->output);

        if (timespec_passed(&next_tick)) {
            clock_gettime(CLOCK_MONOTONIC, &next_tick);
        }

    }
    return NULL;
}
