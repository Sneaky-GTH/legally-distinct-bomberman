#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "server/logic/messagehandling.h"
#include "server/logic/player.h"
#include "server/logic/playingfield.h"
#include "server/net/game.h"
#include "lib/protocol/messages.h"


#define BOMB_COOLDOWN 600


int add_new_client(Client clients[MAX_CLIENTS], ClientMessage* msg) {

    int fd = msg->fd;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].p.id == 0) {
            clients[i].fd = fd;
            clients[i].p.id = i + 1;
            printf("I AM ADDING A NEW CLIENT WITH THE FOLLOWING: %d %d", i, clients[i].p.id);
            clients[i].is_ready = 0;
            clients[i].is_alive = 1;
            strncpy(clients[i].name, msg->msg.data.hello.client_name, 20);
            return i + 1;
        }
    }

    return -1;

}

int remove_client(Client clients[MAX_CLIENTS], int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].p.id == i) {
            clients[i].fd = 0;
            clients[i].is_ready = 0;
            clients[i].is_alive = 0;
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

int srv_process_move_attempt(GameState* game, Message* msg) {

    if (game->clients[msg->sender_id - 1].can_move > 0) return -1;

    printf("Player %d %d %d moving currently is at: %d %d", msg->sender_id, msg->sender_id - 1, game->clients[msg->sender_id - 1].p.id, game->clients[msg->sender_id - 1].p.x, game->clients[msg->sender_id - 1].p.y);

    int res = player_move_attempt(
        &game->wallmap,
        &game->playermap,
        &game->clients[msg->sender_id - 1].p,
        msg->data.move_attempt.direction
    );

    if (res > 0) game->clients[msg->sender_id - 1].can_move = game->default_speed - game->clients[msg->sender_id - 1].p.p_speed * 10;

    return res;

}

int srv_process_bomb_attempt(GameState* game, Message* msg) {

    //if (game->clients[msg->sender_id - 1].can_bomb != 0) return -1;

    int res = player_bomb_attempt(
        game,
        &game->clients[msg->sender_id - 1].p
    );

    if (res > 0) game->clients[msg->sender_id - 1].can_bomb = BOMB_COOLDOWN;

    return res;

}


int srv_process_ready(GameState* game, Message* msg) {

    game->clients[msg->sender_id - 1].is_ready = 1;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id != 0 && game->clients[i].is_ready == 0) {
            return -1;
        }
    }

    game->status = 1;
    printf("GAME INFO: Starting a new game (!!!)\n");
    return 0;

}

