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


void process_bombs(GameState* game) {
    while (game->bombs->lifetime <= 1) {
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


void add_bomb(GameState* game, Bomb* bomb) {
    Bomb* currentbomb = game->bombs;

    if (currentbomb == NULL) {
        game->bombs = bomb;
    }

    while (currentbomb->nextbomb != NULL) {
        currentbomb = currentbomb->nextbomb;
    }

    currentbomb->nextbomb = bomb;
}


int create_bomb(GameState* game, uint8_t target_x, uint8_t target_y, Player* p) {

    Bomb* bomb = malloc(sizeof(Bomb));

    bomb->x = target_x;
    bomb->y = target_y;
    bomb->lifetime = DEFAULT_BOMB_TIMER - p->p_time;
    bomb->radius = 1 + p->p_size;

    add_bomb(game, bomb);
    SAFE_SET_CELL(&game->wallmap, target_x, target_y, 'B');
}


int clear_explosion_from_map(PlayingField* field, uint8_t x, uint8_t y) {
    if (SAFE_GET_CELL(field, x, y) != 'X') {
        return -1;
    }

    SAFE_SET_CELL(field, x, y, '.');
}





