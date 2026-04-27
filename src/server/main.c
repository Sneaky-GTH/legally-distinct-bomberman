#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib.h"
#include <protocol/messages.h>
#include "./game/playingfield.h"
#include <protocol/map.h>

int main(void) {

    struct playingField* field = (struct playingField*)malloc(sizeof(struct playingField));
    init_playingField(field, 7, 5);
    prepare_playingField(field);
    print_playingField(field);

    free_playingField(field);


    return 0;
}
