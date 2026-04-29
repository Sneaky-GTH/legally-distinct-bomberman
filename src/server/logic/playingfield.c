#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "server/logic/playingfield.h"

int init_playingField(PlayingField* field, uint16_t w, uint16_t h) {
    field->width = w;
    field->height = h;

    uint16_t* c = malloc(h * w * sizeof(uint16_t));

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
    for (int i = 1; i < field->height - 1; i++) {
        for (int j = 1; j < field->width - 1; j++) {
            if ((j + i) % 2 == 0) CELL(field, j, i) = (uint8_t)'S';
            if ((j + i) % 3 == 0) CELL(field, j, i) = (uint8_t)'H';
        }
    }

    CELL(field, field->width/2, 0) = (uint8_t)'S';
    CELL(field, field->width/2, field->height - 1) = (uint8_t)'H';
    CELL(field, 0, field->height - 1) = (uint8_t)'H';
    CELL(field, field->width - 1, field->height - 1) = (uint8_t)'H';
}


uint8_t SAFE_GET_CELL(PlayingField* field, uint16_t x, uint16_t y) {
    if (x < 0 || y < 0) {
        return 'f';
    }

    if (x > field->width - 1 || y > field->height - 1) {
        return 'f';
    }

    return CELL(field, x, y);
}

int SAFE_SET_CELL(PlayingField* field, uint16_t x, uint16_t y, uint16_t v) {
    if (x < 0 || y < 0) {
        return -1;
    }

    if (x > field->width - 1 || y > field->height - 1) {
        return -1;
    }

    CELL(field, x, y) = v;
    return 0;
}


int move_cell_contents(PlayingField* field, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    CELL(field, x2, y2) = CELL(field, x1, y1);
    CELL(field, x1, y1) = '.';

    return 0;
}


uint16_t cell_to_uint(PlayingField* field, uint16_t x, uint16_t y) {
    return (y) * (field)->width + (x);
}



