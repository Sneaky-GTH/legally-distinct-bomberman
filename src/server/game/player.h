#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

struct player {
    uint8_t id;
    uint8_t p_size; // bomb explosion size power up count
    uint8_t p_speed; // speed power up count
    uint8_t p_time; // explosion timer power up count
    uint8_t p_count; // bomb count power up count
};

#endif
