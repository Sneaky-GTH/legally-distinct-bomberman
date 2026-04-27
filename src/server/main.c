#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib.h"
#include <protocol/messages.h>
#include "server/game/playingfield.h"
#include "server/game/player.h"
#include <protocol/map.h>

int main(void) {

    struct playingField* field = (struct playingField*)malloc(sizeof(struct playingField));
    init_playingField(field, 7, 5);
    prepare_playingField(field);

    //CELL(field, 0, 0) = '1';
    //move_cell_contents(field, 0, 0, 1, 0);
    struct player* p = init_player(0, 0, 0);
    SAFE_SET_CELL(field, 0, 0, '1');
    player_move_attempt(field, p, DIR_RIGHT);
    player_move_attempt(field, p, DIR_DOWN);
    print_playingField(field);

    free_playingField(field);

    return 0;
}
