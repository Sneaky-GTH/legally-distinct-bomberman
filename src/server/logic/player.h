#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <protocol/messages.h>
#include "server/logic/playingfield.h"
#include "server/logic/bomb.h"

#define MAX_BOMBS 8

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
void reset_player(Player* p, int id, int x, int y) ;

int player_move(PlayingField* field, Player* p,  uint8_t x, uint8_t y);
uint8_t player_move_attempt(PlayingField* ob_field, PlayingField* p_field, Player* p, direction_t dir);
int player_bomb(PlayingField* ob_field, uint8_t target_x, uint8_t target_y);
uint8_t player_bomb_attempt(PlayingField* ob_field, Player* p);

#endif
