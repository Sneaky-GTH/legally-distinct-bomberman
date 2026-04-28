#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "bomb.h"
#include "server/logic/playingfield.h"
#include "server/logic/bomb.h"
#include "server/net/game.h"

int explode_bomb(GameState* game, Bomb bomb) {

    PlayingField* field = &game->wallmap;
    uint8_t x = bomb.x;
    uint8_t y = bomb.y;
    uint8_t r = bomb.radius;
    uint8_t lifetime = bomb.lifetime;

    if (SAFE_GET_CELL(field, x, y) != 'B') {
        return -1;
    }

    SAFE_SET_CELL(field, x, y, 'X');


    for (int i = 1; i < r; i++) {
        if (SAFE_GET_CELL(field, x+i, y) == 'f') break;
        if (SAFE_GET_CELL(field, x+i, y) == 'H') break;
        if (SAFE_GET_CELL(field, x+i, y) == 'S') {
            SAFE_SET_CELL(field, x+i, y, '.');
            break;
        }
        SAFE_SET_CELL(field, x+i, y, 'X');
        Explosion* explo = create_explosion(x+i, y, bomb. lifetime);
        add_explosion(game, explo);
    }

}

Explosion* create_explosion(uint8_t x, uint8_t y, uint8_t lifetime) {
    Explosion* explo = malloc(sizeof(Explosion));
    explo->x = x;
    explo->y = y;
    explo->lifetime = lifetime;
    return explo;
}

void add_explosion(GameState* game, Explosion* explosion) {
    Explosion* currentexplo = game->explosions;

    if (currentexplo == NULL) {
        game->explosions = explosion;
    }

    while (currentexplo->nextexplo != NULL) {
        currentexplo = currentexplo->nextexplo;
    }

    currentexplo->nextexplo = explosion;
}


void process_explosions(GameState* game) {
    while (game->explosions->lifetime <= 1) {
        Explosion* nextex = game->explosions->nextexplo;
        clear_explosion_from_map(&game->wallmap, game->explosions->x, game->explosions->y);
        free(game->explosions);
        game->explosions = nextex;
    }

    Explosion* currentexplo = game->explosions->nextexplo;
    Explosion* prevexplo = game->explosions;
    while (currentexplo != NULL) {
        if (currentexplo->lifetime <= 1) {
            Explosion* nextexplo = currentexplo->nextexplo;
            prevexplo->nextexplo = nextexplo;
            clear_explosion_from_map(&game->wallmap, currentexplo->x, currentexplo->y);
            free(currentexplo);
            currentexplo = nextexplo;
        } else {
            currentexplo->lifetime -= 1;
            currentexplo = currentexplo->nextexplo;
        }
    }

}


int clear_explosion_from_map(PlayingField* field, uint8_t x, uint8_t y) {
    if (SAFE_GET_CELL(field, x, y) != 'X') {
        return -1;
    }

    SAFE_SET_CELL(field, x, y, '.');
}





