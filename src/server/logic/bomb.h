#ifndef BOMB_H
#define BOMB_H

#include <stdint.h>
#include <protocol/messages.h>
#include "playingfield.h"
#include "server/logic/playingfield.h"

typedef struct GameState GameState;

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
int clear_explosion_from_map(PlayingField* field, uint8_t x, uint8_t y);

#endif
