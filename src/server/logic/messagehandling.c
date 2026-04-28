#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "server/logic/messagehandling.h"
#include "server/logic/player.h"
#include "server/logic/playingfield.h"
#include "server/net/game.h"
#include "lib/protocol/messages.h"


int add_new_client(Client clients[MAX_CLIENTS], ClientMessage* msg) {

    int fd = msg->fd;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].p.id == 0) {
            clients[i].fd = fd;
            clients[i].p.id = i + 1;
            clients[i].is_ready = 0;
            strncpy(clients[i].name, msg->msg.data.hello.client_name, 20);
            return i;
        }
    }

    return -1;

}

int remove_client(Client clients[MAX_CLIENTS], int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].p.id == i) {
            clients[i].fd = 0;
            clients[i].is_ready = 0;
            reset_player(&clients[i].p, i, 0, 0);
            return i;
        }
    }

    return -1;
}


int srv_process_hello(GameState* game, ClientMessage* msg) {

    int res = add_new_client(game->clients, msg);
    game->client_count += 1;
    return res;

}


int srv_process_disconnect(GameState* game, int fd) {

    int res = remove_client(game->clients, fd);
    game->client_count -= 1;
    return res;

}

uint8_t srv_process_move_attempt(GameState* game, Message* msg) {

    return player_move_attempt(
        &game->wallmap,
        &game->playermap,
        &game->clients[msg->sender_id].p,
        msg->data.move_attempt.direction
    );

}

uint8_t srv_process_bomb_attempt(GameState* game, Message* msg) {

    return player_bomb_attempt(
        &game->wallmap,
        &game->clients[msg->sender_id].p
    );

}


uint8_t srv_process_ready(GameState* game, Message* msg) {

    game->clients[msg->sender_id].is_ready = 1;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id != 0 && game->clients[i].is_ready == 0) {
            return -1;
        }
    }

    game->status = 1;
    printf("GAME INFO: Starting a new game (!!!)\n");
    return 0;

}

