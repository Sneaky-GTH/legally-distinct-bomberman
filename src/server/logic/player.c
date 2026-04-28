#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "server/logic/player.h"
#include "server/logic/playingfield.h"
#include <protocol/messages.h>

Player* init_player(int id, int x, int y) {
    Player* p = malloc(sizeof(Player*));

    p->id = id;
    p->p_count = 0;
    p->p_size = 0;
    p->p_speed = 0;
    p->p_time = 0;
    p->x = x;
    p->y = y;

    return p;
}

void free_player(Player* p) {
    free(p);
}

uint8_t player_move(PlayingField* field, Player* p, uint8_t x, uint8_t y) {
    move_cell_contents(field, p->x, p->y, x, y);
    p->x = x;
    p->y = y;

    return 't';
}

uint8_t player_move_attempt(PlayingField* field, Player* p, direction_t dir) {

    uint8_t target_x = p->x;
    uint8_t target_y = p->y;

    switch(dir) {
        case DIR_UP:
            target_y -= 1;
            break;
        case DIR_DOWN:
            target_y += 1;
            break;
        case DIR_LEFT:
            target_x -= 1;
            break;
        case DIR_RIGHT:
            target_x += 1;
            break;
        default:
            break;
    }

    if (SAFE_GET_CELL(field, target_x, target_y) != '.') {
        return 'f';
    }

    player_move(field, p, target_x, target_y);

    return 't';

}
