#ifndef PLAYING_FIELD_H
#define PLAYING_FIELD_H

#include <stdint.h>
#include <protocol/messages.h>

#define CELL(field, x, y) (field)->cell[(y) * (field)->width + (x)]

typedef struct PlayingField {
    uint16_t height;
    uint16_t width;
    uint8_t *cell;
} PlayingField;

int init_playingField(PlayingField *field, uint16_t w, uint16_t h);
void print_playingField(PlayingField *field);
void free_playingField(PlayingField *field);
void prepare_playingField(PlayingField *field);

uint8_t SAFE_GET_CELL(PlayingField* field, uint16_t x, uint16_t y);
uint8_t SAFE_SET_CELL(PlayingField* field, uint16_t x, uint16_t y, uint8_t v);
uint8_t move_cell_contents(PlayingField* field, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
uint16_t cell_to_uint(PlayingField* field, uint16_t x, uint16_t y);

#endif
