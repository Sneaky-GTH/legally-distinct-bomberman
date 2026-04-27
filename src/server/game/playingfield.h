#ifndef PLAYING_FIELD_H
#define PLAYING_FIELD_H

#include <stdint.h>

#define CELL(field, x, y) (field)->cell[(y) * (field)->width + (x)]

struct playingField {
    int height;
    int width;
    uint8_t *cell;
};

int init_playingField(struct playingField *field, int w, int h);
void print_playingField(struct playingField *field);
void free_playingField(struct playingField *field);
void prepare_playingField(struct playingField *field);

void print_hello();

#endif
