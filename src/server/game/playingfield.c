#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "./playingfield.h"

int init_playingField(struct playingField* field, int w, int h) {
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

void print_playingField(struct playingField* field) {
    for (int i = 0; i < field->height; i++) {
        for (int j = 0; j < field->width; j++) {
            printf("%c ", CELL(field, j, i));
        }
        printf("\n");
    }
}


void free_playingField(struct playingField* field) {
    free(field->cell);
    free(field);
}

void prepare_playingField(struct playingField *field) {
    for (int i = 0; i < field->height; i++) {
        for (int j = 0; j < field->width; j++) {
            if ((j + i) % 2) CELL(field, j, i) = (uint8_t)'H';
        }
    }
}
