#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <protocol/messages.h>
#include "server/logic/playingfield.h"

typedef struct {
    uint8_t id;
    uint8_t p_size; // bomb explosion size power up count
    uint8_t p_speed; // speed power up count
    uint8_t p_time; // explosion timer power up count
    uint8_t p_count; // bomb count power up
    uint8_t x;
    uint8_t y;
} Player;

Player* init_player(int id, int x, int y);
void free_player(Player* p);

uint8_t player_move_attempt(PlayingField* field, Player* p, direction_t dir);
uint8_t player_move(PlayingField* field, Player* p, uint8_t x, uint8_t y);

#endif
