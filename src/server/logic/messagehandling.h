#ifndef MESSAGE_HANDLING_H
#define MESSAGE_HANDLING_H

#include <stdint.h>
#include <protocol/messages.h>
#include "server/logic/playingfield.h"
#include "server/net/game.h"

#define MAX_CLIENTS 16

void srv_process_hello(GameState* game, Message* msg);
uint8_t srv_process_move_attempt(GameState* game, Message* msg);
uint8_t srv_process_bomb_attempt(GameState* game, Message* msg);

#endif
