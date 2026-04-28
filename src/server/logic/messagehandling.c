#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "server/logic/messagehandling.h"
#include "server/logic/player.h"
#include "server/logic/playingfield.h"
#include "server/net/game.h"
#include "lib/protocol/messages.h"

void srv_process_hello(GameState* game, Message* msg) {

    return;

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

    game->clients[msg->sender_id].ready = 1;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (game->clients[i].p.id != 0 && game->clients[i].ready == 0) {
            return -1;
        }
    }

    game->status = 1;
    return 0;

}

