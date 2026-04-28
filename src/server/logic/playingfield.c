#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "server/logic/playingfield.h"

int init_playingField(PlayingField* field, uint8_t w, uint8_t h) {
    field->width = w;
    field->height = h;

    uint8_t* c = malloc(h * w * sizeof(uint8_t));

    field->cell = c;

    for (int i = 0; i < field->height; i++) {
        for (int j = 0; j < field->width; j++) {
            CELL(field, j, i) = (uint8_t)'.';
        }
    }

    return 1;
}

void print_playingField(PlayingField* field) {
    for (int i = 0; i < field->height; i++) {
        for (int j = 0; j < field->width; j++) {
            printf("%c ", CELL(field, j, i));
        }
        printf("\n");
    }
}


void free_playingField(PlayingField* field) {
    free(field->cell);
    free(field);
}

void prepare_playingField(PlayingField *field) {
    for (int i = 0; i < field->height; i++) {
        for (int j = 0; j < field->width; j++) {
            //if ((j + i) % 2) CELL(field, j, i) = (uint8_t)'H';
        }
    }
}


uint8_t SAFE_GET_CELL(PlayingField* field, uint8_t x, uint8_t y) {
    if (x < 0 || y < 0) {
        return 'f';
    }

    if (x > field->width - 1 || y > field->height - 1) {
        return 'f';
    }

    return CELL(field, x, y);
}

uint8_t SAFE_SET_CELL(PlayingField* field, uint8_t x, uint8_t y, uint8_t v) {
    if (x < 0 || y < 0) {
        return 'f';
    }

    if (x > field->width - 1 || y > field->height - 1) {
        return 'f';
    }

    CELL(field, x, y) = v;
    return 't';
}


uint8_t move_cell_contents(PlayingField* field, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    CELL(field, x2, y2) = CELL(field, x1, y1);
    CELL(field, x1, y1) = '.';

    return 't';
}



