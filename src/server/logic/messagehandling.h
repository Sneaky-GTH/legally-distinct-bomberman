#ifndef MESSAGE_HANDLING_H
#define MESSAGE_HANDLING_H

#include <stdint.h>
#include <protocol/messages.h>
#include "server/logic/playingfield.h"
#include "server/net/game.h"

#define MAX_CLIENTS 16

int add_new_client(Client clients[MAX_CLIENTS], ClientMessage* msg);
int remove_client(Client clients[MAX_CLIENTS], int id);

int srv_process_hello(GameState* game, ClientMessage* msg);
int srv_process_disconnect(GameState* game, int fd);
uint8_t srv_process_move_attempt(GameState* game, Message* msg);
uint8_t srv_process_bomb_attempt(GameState* game, Message* msg);
uint8_t srv_process_ready(GameState* game, Message* msg);
uint8_t srv_start_game(GameState* game);

#endif
