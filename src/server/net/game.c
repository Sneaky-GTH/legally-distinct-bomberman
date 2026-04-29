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

int load_game_state(const char *filename, GameState* game) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;

    int height, width;
    fscanf(fp, "%d %d %d %d %d %d\n",
           &height, &width,
           &game->default_speed,
           &game->default_antibomb,
           &game->default_radius,
           &game->default_countdown);

    game->wallmap.height = (uint16_t)height;
    game->playermap.height = (uint16_t)height;
    game->wallmap.width = (uint16_t)width;
    game->playermap.height = (uint16_t)width;

    size_t cell_count = (size_t)height * width;
    game->wallmap.cell   = calloc(cell_count, sizeof(uint16_t));
    game->playermap.cell = calloc(cell_count, sizeof(uint16_t));

    char line[1024];
    for (int row = 0; row < height; row++) {
        if (!fgets(line, sizeof(line), fp)) break;
        int col = 0;
        for (char *p = line; *p && col < width; p++) {
            if (*p >= '1' && *p <= '9') {
                game->clients[*p - 48 - 1].p.x = row;
                game->clients[*p - 48 - 1].p.y = col;
                continue;
            }
            if (*p != ' ' && *p != '\r' && *p != '\n')
                game->wallmap.cell[row * width + col++] = (uint16_t)(uint8_t)*p;
        }
    }

    fclose(fp);
    return 0;
}


void setup_game(GameState* game) {

    //if (game->status == 1) {
    //    free_playingField(&game->wallmap);
    //    free_playingField(&game->playermap);
    //}

    for (int i = 0; i < MAX_CLIENTS; i++) {
        game->clients[i].fd = 0;
        game->clients[i].p.id = 0;
        game->clients[i].is_ready = 0;
        game->clients[i].is_alive = 1;
        game->clients[i].can_move = 0;
        game->clients[i].can_bomb = 0;
        reset_player(&game->clients[i].p, 0, 255, 255);
    }

    game->bombs = NULL;
    game->explosions = NULL;
    game->status = 0;
    game->client_count = 0;
    game->powerup_counter = 0;
    game->default_speed = 50;
    game->default_antibomb = 60;
    game->default_radius = 1;
    game->default_countdown = 60;
    game->config = 0; //load_game_state("game.cfg", game);

    //if (game->config >= 0) return;

    init_playingField(&game->playermap, 10, 10);
    init_playingField(&game->wallmap, 10, 10);

    prepare_playingField(&game->playermap);
    prepare_playingField(&game->wallmap);

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


void time_down_speed_limit(GameState* game) {

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id == 0) continue;
        if (game->clients[i].is_alive != 1) continue;

        if (game->clients[i].can_move > 0) game->clients[i].can_move -= 5;
        if (game->clients[i].can_bomb > 0) game->clients[i].can_bomb -= 1;

    }

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
    //print_playingField(&game->wallmap);
    //print_playingField(&game->playermap);
    //printf("---------------");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id == 0) continue;
        if (game->clients[i].is_alive != 1) continue;
        if (SAFE_GET_CELL(&game->wallmap, game->clients[i].p.x, game->clients[i].p.y) == 'X') {
            //printf("ALERTA! ALERTA! THIS MF SHOULD BE DEAD!\n");
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


void check_player_powerup(GameState* game, ServerMessage* servermessages) {
    //print_playingField(&game->wallmap);
    //print_playingField(&game->playermap);
    //printf("---------------");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id == 0) continue;
        if (game->clients[i].is_alive != 1) continue;

        uint16_t cell = SAFE_GET_CELL(&game->wallmap, game->clients[i].p.x, game->clients[i].p.y);

        if (cell != 'A' && cell != 'T' && cell != 'N') continue;

        switch (cell) {
            case 'A':
                game->clients[i].p.p_speed += 1;
                break;
            case 'R':
                game->clients[i].p.p_size += 1;
                break;
            case 'T':
                game->clients[i].p.p_time += 1;
                break;
        }

        uint8_t x = game->clients[i].p.x;
        uint8_t y = game->clients[i].p.y;

        SAFE_SET_CELL(&game->wallmap, x, y, '.');

        while (servermessages->nextmsg != NULL) {
            servermessages = servermessages->nextmsg;
        }

        ServerMessage* next = malloc(sizeof(ServerMessage));
        next->nextmsg = NULL;
        servermessages->nextmsg = (struct ServerMessage*)next;

        Message tx_msg = (Message){
            .type = MSG_BONUS_RETRIEVED,
            .sender_id = 255,
            .target_id = 254,
            .data.bonus_retrieved= {
                .player_id = game->clients[i].p.id,
                .position = cell_to_uint(&game->wallmap, x, y)
            }
        };

        servermessages->has_content = 1;
        servermessages->msg = tx_msg;


    }
}


void check_for_winner(GameState* game, ServerMessage* servermessages) {

    // first, check how many are alive
    int alive = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id == 0) continue;
        if (game->clients[i].is_alive == 1) alive += 1;
        if (alive > 1) return;
    }

    // then check who is alive
    alive = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id == 0) continue;
        if (game->clients[i].is_alive == 1) {
            alive = game->clients[i].p.id;
            break;
        }
    }



    while (servermessages->nextmsg != NULL) {
        servermessages = servermessages->nextmsg;
    }

    ServerMessage* next = malloc(sizeof(ServerMessage));
    next->nextmsg = NULL;
    servermessages->nextmsg = (struct ServerMessage*)next;

    Message tx_msg = (Message){
        .type = MSG_WINNER,
        .sender_id = 255,
        .target_id = 254,
        .data.winner= {
            .winner_id = alive,
        }
    };

    servermessages->has_content = 1;
    servermessages->msg = tx_msg;

    setup_game(game);

    gamestate.status = 1;

    next = malloc(sizeof(ServerMessage));
    next->nextmsg = NULL;
    servermessages->nextmsg = (struct ServerMessage*)next;

    tx_msg = (Message){
        .type = MSG_SET_STATUS,
        .sender_id = 255,
        .target_id = 254,
        .data.set_status = {
            .status = 2,
        },
    };

    servermessages->has_content = 1;
    servermessages->msg = tx_msg;


}


void spawn_power_up(GameState* game, ServerMessage* servermessages) {
    game->powerup_counter += 1;

    if (game->powerup_counter < POWERUP_SPAWN_TIME) {
        game->powerup_counter += 10;
        return;
    }

    game->powerup_counter -= POWERUP_SPAWN_TIME;

    int x = rand() % game->wallmap.width;
    int y = rand() % game->wallmap.height;

    if (SAFE_GET_CELL(&game->wallmap, x, y) != '.') return;

    int type = rand() % 4;
    uint8_t powerup;

    switch (type) {
        case 0:
            powerup = 'A';
            break;
        case 1:
            powerup = 'R';
            break;
        case 2:
            powerup = 'T';
            break;
    }

    SAFE_SET_CELL(&game->wallmap, x, y, powerup);


    while (servermessages->nextmsg != NULL) {
        servermessages = servermessages->nextmsg;
    }

    ServerMessage* next = malloc(sizeof(ServerMessage));
    next->nextmsg = NULL;
    servermessages->nextmsg = (struct ServerMessage*)next;

    Message tx_msg = (Message){
        .type = MSG_BONUS_AVAILABLE,
        .sender_id = 255,
        .target_id = 254,
        .data.bonus_available= {
            .bonus_type = type,
            .position = cell_to_uint(&game->wallmap, x, y),
        }
    };

    servermessages->has_content = 1;
    servermessages->msg = tx_msg;


}


void spread_out_players(GameState* game, MessageQueue* output) {

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id == 0) continue;

        uint8_t res;

        switch(game->clients[i].p.id) {
            case 1:
                printf("Yep, were adding the client here...\n");
                res = player_set_spawn(&game->playermap, &game->clients[i].p, 0, 0, '1' + i);
                break;
            case 2:
                res = player_set_spawn(&game->playermap, &game->clients[i].p, game->playermap.width - 1, 0, '1' + i);
                break;
            case 3:
                res = player_set_spawn(&game->playermap, &game->clients[i].p, 0, game->playermap.height - 1, '1' + i);
                break;
            case 4:
                res = player_set_spawn(&game->playermap, &game->clients[i].p, game->playermap.width - 1, game->playermap.height - 1, '1' + i);
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


void spawn_players(GameState* game, MessageQueue* output) {

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id == 0) continue;

        uint8_t res;

        res = player_set_spawn(&game->playermap, &game->clients[i].p, game->clients[i].p.x, game->clients[i].p.y, '1' + i);

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
    int res;

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
                    .new_position = (uint16_t)res
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

            printf("%d\n", gamestate.config);
            spread_out_players(&gamestate, output);
            /*if (gamestate.config <= 0) {
                spread_out_players(&gamestate, output);
            } else {
                spawn_players(&gamestate, output);
            }*/

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


void gametick(GameState* game, MessageQueue* output) {

    //printf("CHECK: %d\n", game->clients[0].p.id);

    if(game->status != 1) return;

    ServerMessage* servermessages = malloc(sizeof(ServerMessage));
    servermessages->nextmsg = NULL;
    servermessages->has_content = 0;

    //printf("GAME INFO: Processing tick...\n");

    check_player_death(game, servermessages);
    process_bombs(game, servermessages);
    process_antibombs(game, servermessages);
    process_explosions(game);
    check_for_winner(game, servermessages);
    spawn_power_up(game, servermessages);
    time_down_speed_limit(game);

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
    gamestate.status = 0;
    setup_game(&gamestate);

    struct timespec next_tick;
    clock_gettime(CLOCK_MONOTONIC, &next_tick);

    while (1) {

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

        if (timespec_passed(&next_tick)) {
            gametick(&gamestate, args->output);
            next_tick.tv_nsec += 16L * 1000000L * 2L;
            if (next_tick.tv_nsec >= 1000000000L) {
                next_tick.tv_sec  += 1;
                next_tick.tv_nsec -= 1000000000L;
            }
        }

    }
    return NULL;
}
