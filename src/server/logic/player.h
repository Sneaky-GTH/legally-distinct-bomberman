#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <protocol/messages.h>
#include "server/logic/playingfield.h"
#include "server/logic/bomb.h"

#define MAX_BOMBS 8

typedef struct Player {
    uint8_t id;
    uint8_t p_size; // bomb explosion size power up count
    uint8_t p_speed; // speed power up count
    uint8_t p_time; // explosion timer power up count
    uint8_t p_count; // bomb count power up
    uint16_t x;
    uint16_t y;
} Player;

Player* init_player(int id, uint8_t x, uint8_t y);
void free_player(Player* p);
void reset_player(Player* p, int id, uint8_t x, uint8_t y) ;

int player_move(PlayingField* field, Player* p,  uint8_t x, uint8_t y);
int player_set_spawn(PlayingField* field, Player* p, uint8_t x, uint8_t y, uint8_t c);
int player_move_attempt(PlayingField* ob_field, PlayingField* p_field, Player* p, direction_t dir);
int player_bomb_attempt(GameState* game, Player* p);

#endif
