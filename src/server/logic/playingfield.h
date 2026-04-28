#ifndef PLAYING_FIELD_H
#define PLAYING_FIELD_H

#include <stdint.h>
#include <protocol/messages.h>

#define CELL(field, x, y) (field)->cell[(y) * (field)->width + (x)]

typedef struct {
    int height;
    int width;
    uint8_t *cell;
} PlayingField;

int init_playingField(PlayingField *field, uint8_t w, uint8_t h);
void print_playingField(PlayingField *field);
void free_playingField(PlayingField *field);
void prepare_playingField(PlayingField *field);

uint8_t SAFE_GET_CELL(PlayingField* field, uint8_t x, uint8_t y);
uint8_t SAFE_SET_CELL(PlayingField* field, uint8_t x, uint8_t y, uint8_t v);
uint8_t move_cell_contents(PlayingField* field, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

#endif
