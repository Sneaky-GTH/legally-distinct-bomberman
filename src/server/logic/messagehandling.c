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

