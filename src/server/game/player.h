#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <protocol/messages.h>
#include "server/game/playingfield.h"

struct player {
    uint8_t id;
    uint8_t p_size; // bomb explosion size power up count
    uint8_t p_speed; // speed power up count
    uint8_t p_time; // explosion timer power up count
    uint8_t p_count; // bomb count power up
    uint8_t x;
    uint8_t y;
};

struct player* init_player(int id, int x, int y);
void free_player(struct player* p);

uint8_t player_move_attempt(struct playingField* field, struct player* p, direction_t dir);
uint8_t player_move(struct playingField* field, struct player* p, uint8_t x, uint8_t y);

#endif
