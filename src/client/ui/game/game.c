#include "../../game/state.h"
#include "./assets/sprites.h"
#include "./game.h"
#include <GL/glut.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define POS(x, y) ((y) * game_state->width + (x))

void draw_game_board() {
    bind_spritesheet(); // Ensure texture is bound for drawing

    const struct GameState *game_state = get_game_state();

    // Calculate the size of each tile based on the window size and the map size, so that the whole map fits in the window
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);

    float tile_width = (float)window_width / game_state->width;
    float tile_height = (float)window_height / game_state->height;

    // Square tiles, so use the smaller of the two dimensions to avoid stretching
    float tile_size = tile_width < tile_height ? tile_width : tile_height;

    // Center the map in the window
    float x_offset = (window_width - tile_size * game_state->width) / 2;
    float y_offset = (window_height - tile_size * game_state->height) / 2;

    // There are 7 tiles for the background, in a gradient.
    // Every second tile is a gradient tile that should be used when transitioning between two background colors.
    // 3 gradient tiles, 4 solid color tiles
    float change_background_every = game_state->height / 7.0f;

    int bg_tile = 1; // 1 - 7

    // Draw background first
    for (int y = 0; y < game_state->height; y++) {
        if (y >= change_background_every * bg_tile && bg_tile < 7) {
            bg_tile = (y / change_background_every) + 1;
        }

        SpriteId bg_sprite;

        switch (bg_tile) {
            case 1: bg_sprite = SPRITE_BG_BLUE_1; break;
            case 2: bg_sprite = SPRITE_BG_BLUE_12; break;
            case 3: bg_sprite = SPRITE_BG_BLUE_2; break;
            case 4: bg_sprite = SPRITE_BG_BLUE_23; break;
            case 5: bg_sprite = SPRITE_BG_BLUE_3; break;
            case 6: bg_sprite = SPRITE_BG_BLUE_34; break;
            case 7: bg_sprite = SPRITE_BG_BLUE_4; break;
            default: bg_sprite = SPRITE_BG_BLUE_1; break; // UNREACHABLE
        }
        
        for (int x = 0; x < game_state->width; x++) {
            float x_pos = x_offset + x * tile_size;
            float y_pos = y_offset + y * tile_size;

            draw_sprite(bg_sprite, x_pos, y_pos, tile_size, tile_size);
        }

        if (bg_tile % 2 == 0) {
            // This is a gradient tile, so do not draw two of them in a row
            bg_tile++;
        }
    }

    for (int y = 0; y < game_state->height; y++) {
        for (int x = 0; x < game_state->width; x++) {
            cell_types_t cell = game_state->field[POS(x, y)];
            SpriteId spr = SPRITE_NONE;

            switch (cell) {
                case HARD_WALL:
                    spr = SPRITE_DIRT;
                    if (y == 0 || game_state->field[POS(x, y - 1)] != HARD_WALL) {
                        spr = SPRITE_DIRT_TOP;
                    }
                    break;
                case SOFT_WALL:
                    // Randomly (deterministic based on position) choose a soft wall sprite for variety
                    spr = (x * 31 + y * 17) % 2 == 0 ? SPRITE_BROKEN_DIRT_1 : SPRITE_BROKEN_DIRT_2;
                    break;
                case BOMB:
                    spr = SPRITE_BOMB;
                    break;
                case EXPLOSION:
                    spr = SPRITE_NONE; // TODO: add explosion sprite(s)
                    break;
                case POWERUP_SPEED:
                    spr = SPRITE_BONUS_SPEED;
                    break;
                case POWERUP_SIZE:
                    spr = SPRITE_BONUS_SIZE;
                    break;
                case POWERUP_TIME:
                    spr = SPRITE_BONUS_TIME;
                    break;
                case POWERUP_COUNT:
                    spr = SPRITE_BONUS_COUNT;
                    break;
                case EMPTY:
                default:
                    spr = SPRITE_NONE;
                    break;
            }

            float x_pos = x_offset + x * tile_size;
            float y_pos = y_offset + y * tile_size;

            draw_sprite(spr, x_pos, y_pos, tile_size, tile_size);
        }
    }

    for (int i = 0; i < game_state->num_players; i++) {
        const struct Player *player = &game_state->players[i];
        if (!player->alive) {
            continue;
        }

        SpriteId pid = SPRITE_SANDSTONE; // TODO: add player sprites

        float x_pos = x_offset + player->x * tile_size;
        float y_pos = y_offset + player->y * tile_size;

        // Draw player sprite
        draw_sprite(pid, x_pos, y_pos, tile_size, tile_size);
    }
}
