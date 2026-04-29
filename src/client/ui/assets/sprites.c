#include "./sprites.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GLuint spritesheet_tex = 0;
static SpriteDef sprite_defs[SPRITE_COUNT];

// Loads a raw RGBA file using known dimensions
static unsigned char* load_rgba(const char* rgba_filename, int width, int height) {
    FILE* f = fopen(rgba_filename, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open %s\n", rgba_filename);
        return NULL;
    }
    
    int data_size = width * height * 4;
    unsigned char* data = malloc(data_size);
    if (!data) {
        fclose(f);
        return NULL;
    }
    
    // Read the raw RGBA pixels
    size_t read_bytes = fread(data, 1, data_size, f);
    if (read_bytes != (size_t)data_size) {
        fprintf(stderr, "Warning: Expected %d bytes but read %zu bytes in %s\n", data_size, read_bytes, rgba_filename);
    }
    fclose(f);
    
    return data;
}

// Maps a slice of the spritesheet to a sprite ID
static void define_sprite(SpriteId id, float px_x, float px_y, float px_w, float px_h, float tex_w, float tex_h) {
    if (id >= SPRITE_COUNT) return;
    if (id == SPRITE_NONE) return; // No need to define UVs for the "none" sprite
    
    float u_min = px_x / tex_w;
    float u_max = (px_x + px_w) / tex_w;
    // Compute V coords: invert Y assuming tileset is top-left oriented visually but GL is bottom-left
    float v_max = 1.0f - (px_y / tex_h); // Top Y
    float v_min = 1.0f - ((px_y + px_h) / tex_h); // Bottom Y
    
    sprite_defs[id].u_min = u_min;
    sprite_defs[id].v_min = v_min;
    sprite_defs[id].u_max = u_max;
    sprite_defs[id].v_max = v_max;
}

// Ensure alpha blending is enabled
void enable_blending() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void init_sprites(void) {
    if (spritesheet_tex != 0) return;

    // We already know the size of the spritesheet
    int w = 256;
    int h = 256;
    unsigned char* pixels = load_rgba("assets/world_tileset.rgba", w, h);
    if (!pixels) {
        fprintf(stderr, "Error: Could not load assets/world_tileset.rgba\n");
        return;
    }

    glGenTextures(1, &spritesheet_tex);
    glBindTexture(GL_TEXTURE_2D, spritesheet_tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Pass the raw RGBA data direct to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    free(pixels);

    enable_blending();

    float tex_w = (float)w;
    float tex_h = (float)h;

#define SPRITE(id, x, y) define_sprite(SPRITE_##id, (x) * 16, tex_h - (y + 1) * 16, 16, 16, tex_w, tex_h)
    // WALLS
    SPRITE(DIRT_TOP,           0, 0);
    SPRITE(DIRT,               0, 1);
    SPRITE(BROKEN_DIRT_1,      1, 0);
    SPRITE(BROKEN_DIRT_2,      1, 1);
    SPRITE(SANDSTONE_TOP,      2, 0);
    SPRITE(SANDSTONE,          2, 1);
    SPRITE(BROKEN_SANDSTONE_1, 3, 0);
    SPRITE(BROKEN_SANDSTONE_2, 3, 1);
    SPRITE(GRANITE_TOP,        4, 0);
    SPRITE(GRANITE,            4, 1);
    SPRITE(BROKEN_GRANITE_1,   5, 0);
    SPRITE(BROKEN_GRANITE_2,   5, 1);
    SPRITE(STONE_TOP,          6, 0);
    SPRITE(STONE,              6, 1);
    SPRITE(BROKEN_STONE_1,     7, 0);
    SPRITE(BROKEN_STONE_2,     7, 1);

    // MISC
    SPRITE(BOMB,        9, 0);
    SPRITE(BOMB_ALT,    9, 1);
    SPRITE(BONUS_SIZE,  0, 5);
    SPRITE(BONUS_SPEED, 0, 6);
    SPRITE(BONUS_TIME,  1, 5);
    SPRITE(BONUS_COUNT, 1, 6);

    // PLAYERS
    SPRITE(PLAYER_1, 2, 5);
    SPRITE(PLAYER_2, 2, 6);
    SPRITE(PLAYER_3, 2, 7);
    SPRITE(PLAYER_4, 2, 8);
    SPRITE(PLAYER_5, 3, 5);
    SPRITE(PLAYER_6, 3, 6);
    SPRITE(PLAYER_7, 3, 7);
    SPRITE(PLAYER_8, 3, 8);

    // BACKGROUND TILES
    SPRITE(BG_BLUE_1,    0, 9);
    SPRITE(BG_BLUE_12,   0, 10);
    SPRITE(BG_BLUE_2,    0, 11);
    SPRITE(BG_BLUE_23,   0, 12);
    SPRITE(BG_BLUE_3,    0, 13);
    SPRITE(BG_BLUE_34,   0, 14);
    SPRITE(BG_BLUE_4,    0, 15);
    SPRITE(BG_YELLOW_1,  1, 9);
    SPRITE(BG_YELLOW_12, 1, 10);
    SPRITE(BG_YELLOW_2,  1, 11);
    SPRITE(BG_YELLOW_23, 1, 12);
    SPRITE(BG_YELLOW_3,  1, 13);
    SPRITE(BG_YELLOW_34, 1, 14);
    SPRITE(BG_YELLOW_4,  1, 15);
    SPRITE(BG_PURPLE_1,  2, 9);
    SPRITE(BG_PURPLE_12, 2, 10);
    SPRITE(BG_PURPLE_2,  2, 11);
    SPRITE(BG_PURPLE_23, 2, 12);
    SPRITE(BG_PURPLE_3,  2, 13);
    SPRITE(BG_PURPLE_34, 2, 14);
    SPRITE(BG_PURPLE_4,  2, 15);
    SPRITE(BG_GRAY_1,    3, 9);
    SPRITE(BG_GRAY_12,   3, 10);
    SPRITE(BG_GRAY_2,    3, 11);
    SPRITE(BG_GRAY_23,   3, 12);
    SPRITE(BG_GRAY_3,    3, 13);
    SPRITE(BG_GRAY_34,   3, 14);
    SPRITE(BG_GRAY_4,    3, 15);

    // EXPLOSIONS
    SPRITE(EXPLOSION_ULR, 0, 2);
    SPRITE(EXPLOSION_UDL, 1, 2);
    SPRITE(EXPLOSION_UD,  2, 2);
    SPRITE(EXPLOSION_UDR, 3, 2);
    SPRITE(EXPLOSION,     4, 2);
    SPRITE(EXPLOSION_LR,  0, 3);
    SPRITE(EXPLOSION_DR,  1, 3);
    SPRITE(EXPLOSION_DL,  2, 3);
    SPRITE(EXPLOSION_D,   3, 3);
    SPRITE(EXPLOSION_U,   4, 3);
    SPRITE(EXPLOSION_DLR, 0, 4);
    SPRITE(EXPLOSION_UR,  1, 4);
    SPRITE(EXPLOSION_UL,  2, 4);
    SPRITE(EXPLOSION_L,   3, 4);
    SPRITE(EXPLOSION_R,   4, 4);
    SPRITE(EXPLOSION_ALT_ULR, 5, 2);
    SPRITE(EXPLOSION_ALT_UDL, 6, 2);
    SPRITE(EXPLOSION_ALT_UD,  7, 2);
    SPRITE(EXPLOSION_ALT_UDR, 8, 2);
    SPRITE(EXPLOSION_ALT,     9, 2);
    SPRITE(EXPLOSION_ALT_LR,  5, 3);
    SPRITE(EXPLOSION_ALT_DR,  6, 3);
    SPRITE(EXPLOSION_ALT_DL,  7, 3);
    SPRITE(EXPLOSION_ALT_D,   8, 3);
    SPRITE(EXPLOSION_ALT_U,   9, 3);
    SPRITE(EXPLOSION_ALT_DLR, 5, 4);
    SPRITE(EXPLOSION_ALT_UR,  6, 4);
    SPRITE(EXPLOSION_ALT_UL,  7, 4);
    SPRITE(EXPLOSION_ALT_L,   8, 4);
    SPRITE(EXPLOSION_ALT_R,   9, 4);
    SPRITE(BG_DIRT_TL,      4, 5);
    SPRITE(BG_DIRT_TM,      5, 5);
    SPRITE(BG_DIRT_TR,      6, 5);
    SPRITE(BG_DIRT_HL,      4, 6);
    SPRITE(BG_DIRT_HM,      5, 6);
    SPRITE(BG_DIRT_HR,      6, 6);
    SPRITE(BG_DIRT_ML,      4, 7);
    SPRITE(BG_DIRT_MM,      5, 7);
    SPRITE(BG_DIRT_MR,      6, 7);
    SPRITE(BG_DIRT_LL,      4, 8);
    SPRITE(BG_DIRT_LM,      5, 8);
    SPRITE(BG_DIRT_LR,      6, 8);
    SPRITE(BG_DIRT_BL,      4, 9);
    SPRITE(BG_DIRT_BM,      5, 9);
    SPRITE(BG_DIRT_BR,      6, 9);
    SPRITE(BG_SANDSTONE_TL, 7, 5);
    SPRITE(BG_SANDSTONE_TM, 8, 5);
    SPRITE(BG_SANDSTONE_TR, 9, 5);
    SPRITE(BG_SANDSTONE_HL, 7, 6);
    SPRITE(BG_SANDSTONE_HM, 8, 6);
    SPRITE(BG_SANDSTONE_HR, 9, 6);
    SPRITE(BG_SANDSTONE_ML, 7, 7);
    SPRITE(BG_SANDSTONE_MM, 8, 7);
    SPRITE(BG_SANDSTONE_MR, 9, 7);
    SPRITE(BG_SANDSTONE_LL, 7, 8);
    SPRITE(BG_SANDSTONE_LM, 8, 8);
    SPRITE(BG_SANDSTONE_LR, 9, 8);
    SPRITE(BG_SANDSTONE_BL, 7, 9);
    SPRITE(BG_SANDSTONE_BM, 8, 9);
    SPRITE(BG_SANDSTONE_BR, 9, 9);
    SPRITE(BG_GRANITE_TL,   4, 10);
    SPRITE(BG_GRANITE_TM,   5, 10);
    SPRITE(BG_GRANITE_TR,   6, 10);
    SPRITE(BG_GRANITE_HL,   4, 11);
    SPRITE(BG_GRANITE_HM,   5, 11);
    SPRITE(BG_GRANITE_HR,   6, 11);
    SPRITE(BG_GRANITE_ML,   4, 12);
    SPRITE(BG_GRANITE_MM,   5, 12);
    SPRITE(BG_GRANITE_MR,   6, 12);
    SPRITE(BG_GRANITE_LL,   4, 13);
    SPRITE(BG_GRANITE_LM,   5, 13);
    SPRITE(BG_GRANITE_LR,   6, 13);
    SPRITE(BG_GRANITE_BL,   4, 14);
    SPRITE(BG_GRANITE_BM,   5, 14);
    SPRITE(BG_GRANITE_BR,   6, 14);
    SPRITE(BG_STONE_TL,     7, 10);
    SPRITE(BG_STONE_TM,     8, 10);
    SPRITE(BG_STONE_TR,     9, 10);
    SPRITE(BG_STONE_HL,     7, 11);
    SPRITE(BG_STONE_HM,     8, 11);
    SPRITE(BG_STONE_HR,     9, 11);
    SPRITE(BG_STONE_ML,     7, 12);
    SPRITE(BG_STONE_MM,     8, 12);
    SPRITE(BG_STONE_MR,     9, 12);
    SPRITE(BG_STONE_LL,     7, 13);
    SPRITE(BG_STONE_LM,     8, 13);
    SPRITE(BG_STONE_LR,     9, 13);
    SPRITE(BG_STONE_BL,     7, 14);
    SPRITE(BG_STONE_BM,     8, 14);
    SPRITE(BG_STONE_BR,     9, 14);

#undef SPRITE

    // Non-standard
    define_sprite(SPRITE_BUTTON_LEFT,   128, tex_h - 16, 6, 16, tex_w, tex_h);
    define_sprite(SPRITE_BUTTON_MIDDLE, 134, tex_h - 16, 4, 16, tex_w, tex_h);
    define_sprite(SPRITE_BUTTON_RIGHT,  138, tex_h - 16, 6, 16, tex_w, tex_h);

    define_sprite(SPRITE_INPUT_LEFT,   161, tex_h - 21, 6, 21, tex_w, tex_h);
    define_sprite(SPRITE_INPUT_MIDDLE, 167, tex_h - 21, 1, 21, tex_w, tex_h);
    define_sprite(SPRITE_INPUT_RIGHT,  168, tex_h - 21, 8, 21, tex_w, tex_h);

    define_sprite(SPRITE_TEXTBOX_TOPLEFT,     160, tex_h - 25, 3, 4, tex_w, tex_h);
    define_sprite(SPRITE_TEXTBOX_TOP,         163, tex_h - 25, 1, 4, tex_w, tex_h);
    define_sprite(SPRITE_TEXTBOX_TOPRIGHT,    164, tex_h - 25, 3, 4, tex_w, tex_h);
    define_sprite(SPRITE_TEXTBOX_MIDDLELEFT,  160, tex_h - 27, 3, 1, tex_w, tex_h);
    define_sprite(SPRITE_TEXTBOX_MIDDLE,      163, tex_h - 27, 1, 1, tex_w, tex_h);
    define_sprite(SPRITE_TEXTBOX_MIDDLERIGHT, 164, tex_h - 27, 3, 1, tex_w, tex_h);
    define_sprite(SPRITE_TEXTBOX_BOTTOMLEFT,  160, tex_h - 30, 3, 4, tex_w, tex_h);
    define_sprite(SPRITE_TEXTBOX_BOTTOM,      163, tex_h - 30, 1, 4, tex_w, tex_h);
    define_sprite(SPRITE_TEXTBOX_BOTTOMRIGHT, 164, tex_h - 30, 3, 4, tex_w, tex_h);
}

SpriteDef get_sprite_def(SpriteId id) {
    if (id < 0 || id >= SPRITE_COUNT) return sprite_defs[0];
    return sprite_defs[id];
}

void bind_spritesheet(void) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, spritesheet_tex);
}

void unbind_spritesheet(void) {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void draw_sprite(SpriteId id, float x, float y, float w, float h) {
    if (id >= SPRITE_COUNT) return;
    if (id == SPRITE_NONE) return; // Don't draw anything for the "none" sprite

    SpriteDef def = get_sprite_def(id);
    
    // Draw quad with texture mapped correctly
    glBegin(GL_QUADS);
    glTexCoord2f(def.u_min, def.v_min); glVertex2f(x,     y);
    glTexCoord2f(def.u_max, def.v_min); glVertex2f(x + w, y);
    glTexCoord2f(def.u_max, def.v_max); glVertex2f(x + w, y + h);
    glTexCoord2f(def.u_min, def.v_max); glVertex2f(x,     y + h);
    glEnd();
}

void blit_textbox(float x, float y, float width, float height) {
    bind_spritesheet();

    float m = 2.0f; // Scale up the 3px corners to 6px for better visibility

    float mw = width - (6 * m); // 3px for each side
    float mh = height - (8 * m); // 4px for each side

    // Top row
    draw_sprite(SPRITE_TEXTBOX_TOPLEFT, x, y, 3 * m, 4 * m);
    draw_sprite(SPRITE_TEXTBOX_TOP, x + 3 * m, y, mw, 4 * m);
    draw_sprite(SPRITE_TEXTBOX_TOPRIGHT, x + 3 * m + mw, y, 3 * m, 4 * m);
    // Middle row
    draw_sprite(SPRITE_TEXTBOX_MIDDLELEFT, x, y + 4 * m, 3 * m, mh);
    draw_sprite(SPRITE_TEXTBOX_MIDDLE, x + 3 * m, y + 4 * m, mw, mh);
    draw_sprite(SPRITE_TEXTBOX_MIDDLERIGHT, x + 3 * m + mw, y + 4 * m, 3 * m, mh);
    // Bottom row
    draw_sprite(SPRITE_TEXTBOX_BOTTOMLEFT, x, y + 4 * m + mh, 3 * m, 4 * m);
    draw_sprite(SPRITE_TEXTBOX_BOTTOM, x + 3 * m, y + 4 * m + mh, mw, 4 * m);
    draw_sprite(SPRITE_TEXTBOX_BOTTOMRIGHT, x + 3 * m + mw, y + 4 * m + mh, 3 * m, 4 * m);

    unbind_spritesheet();
}
