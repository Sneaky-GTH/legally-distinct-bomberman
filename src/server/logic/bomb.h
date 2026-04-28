#ifndef BOMB_H
#define BOMB_H

#include <stdint.h>
#include <protocol/messages.h>
#include "server/logic/playingfield.h"

#define DEFAULT_BOMB_TIMER 10

typedef struct GameState GameState;
typedef struct Player Player;

typedef struct Bomb {
    uint8_t x;
    uint8_t y;
    uint8_t radius;
    uint8_t lifetime;
    struct Bomb* nextbomb;
} Bomb;

typedef struct Antibomb {
    uint8_t x;
    uint8_t y;
    uint8_t radius;
    uint8_t lifetime;
    struct Antibomb* nextantibomb;
} Antibomb;

typedef struct Explosion {
    uint8_t x;
    uint8_t y;
    uint8_t lifetime;
    struct Explosion* nextexplo;
} Explosion;

int explode_bomb(GameState* game, Bomb bomb);
Explosion* create_explosion(uint8_t x, uint8_t y, uint8_t lifetime);
void add_explosion(GameState* game, Explosion* explosion);
void process_explosions(GameState* game);
void process_bombs(GameState* game);
void process_antibombs(GameState* game);
int create_bomb(GameState* game, uint8_t target_x, uint8_t target_y, Player* p);
int clear_explosion_from_map(PlayingField* field, uint8_t x, uint8_t y);

#endif
