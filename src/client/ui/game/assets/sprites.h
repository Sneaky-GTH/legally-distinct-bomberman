#pragma once
#include <GL/glut.h>

typedef struct {
    float u_min;
    float v_min;
    float u_max;
    float v_max;
} SpriteDef;


typedef enum {
    SPRITE_NONE,
    SPRITE_DIRT_TOP,
    SPRITE_DIRT,
    SPRITE_BROKEN_DIRT_1,
    SPRITE_BROKEN_DIRT_2,
    SPRITE_SANDSTONE_TOP,
    SPRITE_SANDSTONE,
    SPRITE_BROKEN_SANDSTONE_1,
    SPRITE_BROKEN_SANDSTONE_2,
    SPRITE_GRANITE_TOP,
    SPRITE_GRANITE,
    SPRITE_BROKEN_GRANITE_1,
    SPRITE_BROKEN_GRANITE_2,
    SPRITE_STONE_TOP,
    SPRITE_STONE,
    SPRITE_BROKEN_STONE_1,
    SPRITE_BROKEN_STONE_2,
    SPRITE_BONUS_SPEED,
    SPRITE_BONUS_SIZE,
    SPRITE_BONUS_TIME,
    SPRITE_BONUS_COUNT,
    SPRITE_BOMB,
    SPRITE_BG_BLUE_1,
    SPRITE_BG_BLUE_12,
    SPRITE_BG_BLUE_2,
    SPRITE_BG_BLUE_23,
    SPRITE_BG_BLUE_3,
    SPRITE_BG_BLUE_34,
    SPRITE_BG_BLUE_4,
    SPRITE_BG_YELLOW_1,
    SPRITE_BG_YELLOW_12,
    SPRITE_BG_YELLOW_2,
    SPRITE_BG_YELLOW_23,
    SPRITE_BG_YELLOW_3,
    SPRITE_BG_YELLOW_34,
    SPRITE_BG_YELLOW_4,
    SPRITE_BG_PURPLE_1,
    SPRITE_BG_PURPLE_12,
    SPRITE_BG_PURPLE_2,
    SPRITE_BG_PURPLE_23,
    SPRITE_BG_PURPLE_3,
    SPRITE_BG_PURPLE_34,
    SPRITE_BG_PURPLE_4,
    SPRITE_BG_GRAY_1,
    SPRITE_BG_GRAY_12,
    SPRITE_BG_GRAY_2,
    SPRITE_BG_GRAY_23,
    SPRITE_BG_GRAY_3,
    SPRITE_BG_GRAY_34,
    SPRITE_BG_GRAY_4,
    // Keep adding new sprites here!
    SPRITE_COUNT,
} SpriteId;


// Initialize the BMP loader and texture
void init_sprites(void);

// Given an ID, get the UV coordinates for custom rendering
SpriteDef get_sprite_def(SpriteId id);

// Call before rendering multiple sprites (or simply use draw_sprite which does it per element)
void bind_spritesheet(void);

// Easily blit a sprite to the screen with its UV dimensions properly calculated
void draw_sprite(SpriteId id, float x, float y, float w, float h);
