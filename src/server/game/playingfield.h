#ifndef PLAYING_FIELD_H
#define PLAYING_FIELD_H

#include <stdint.h>
#include <protocol/messages.h>

#define CELL(field, x, y) (field)->cell[(y) * (field)->width + (x)]

struct playingField {
    int height;
    int width;
    uint8_t *cell;
};

int init_playingField(struct playingField *field, uint8_t w, uint8_t h);
void print_playingField(struct playingField *field);
void free_playingField(struct playingField *field);
void prepare_playingField(struct playingField *field);

uint8_t SAFE_GET_CELL(struct playingField* field, uint8_t x, uint8_t y);
uint8_t SAFE_SET_CELL(struct playingField* field, uint8_t x, uint8_t y, uint8_t v);
uint8_t move_cell_contents(struct playingField* field, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

#endif
