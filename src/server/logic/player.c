#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "server/logic/player.h"
#include "playingfield.h"
#include "server/logic/playingfield.h"
#include "server/logic/bomb.h"
#include <protocol/messages.h>
#include "server/net/game.h"

Player* init_player(int id, uint8_t x, uint8_t y) {
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

void reset_player(Player* p, int id, uint8_t x, uint8_t y) {
    p->id = id;
    p->p_count = 0;
    p->p_size = 0;
    p->p_speed = 0;
    p->p_time = 0;
    p->x = x;
    p->y = y;
}

int player_move(PlayingField* field, Player* p, uint8_t x, uint8_t y) {
    move_cell_contents(field, p->x, p->y, x, y);
    p->x = x;
    p->y = y;

    return 0;
}

uint8_t player_move_attempt(PlayingField* ob_field, PlayingField* p_field, Player* p, direction_t dir) {

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

    if (SAFE_GET_CELL(ob_field, target_x, target_y) != '.') {
        return -1;
    }

    player_move(p_field, p, target_x, target_y);

    return cell_to_uint(p_field, target_x, target_y);

}


uint8_t player_bomb_attempt(GameState* game, Player* p) {

    uint8_t target_x = p->x;
    uint8_t target_y = p->y;

    if (SAFE_GET_CELL(&game->wallmap, target_x, target_y) != '.') {
        return -1;
    }

    create_bomb(game, target_x, target_y, p);

    return cell_to_uint(&game->wallmap, target_x, target_y);

}
