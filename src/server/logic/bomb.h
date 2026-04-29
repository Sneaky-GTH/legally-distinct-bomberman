#ifndef BOMB_H
#define BOMB_H

#include <stdint.h>
#include <protocol/messages.h>
#include "server/logic/playingfield.h"
#include "server/net/args.h"

#define DEFAULT_BOMB_TIMER 10

typedef struct GameState GameState;
typedef struct Player Player;

typedef struct Bomb {
    uint16_t x;
    uint16_t y;
    uint8_t radius;
    uint8_t lifetime;
    struct Bomb* nextbomb;
} Bomb;

typedef struct Antibomb {
    uint16_t x;
    uint16_t y;
    uint8_t radius;
    uint8_t lifetime;
    struct Antibomb* nextantibomb;
} Antibomb;

typedef struct Explosion {
    uint16_t x;
    uint16_t y;
    uint8_t lifetime;
    struct Explosion* nextexplo;
} Explosion;

int explode_bomb(GameState* game, Bomb* bomb, MessageQueue* queue);
Explosion* create_explosion(uint8_t x, uint8_t y, Bomb* bomb);
void add_explosion(GameState* game, Explosion* explosion);
void process_explosions(GameState* game);
void process_bombs(GameState* game, MessageQueue* queue);
void process_antibombs(GameState* game);
int create_bomb(GameState* game, uint8_t target_x, uint8_t target_y, Player* p);
int clear_explosion_from_map(PlayingField* field, uint8_t x, uint8_t y);

#endif
