#include "../../config/config.h"
#include "../../game/state.h"
#include "./assets/themes.h"
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
    const struct ThemedSprites *theme = get_current_theme();

    glColor4f(1.0, 1.0, 1.0, 1.0); // Ensure full opacity and no tint

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

    // Draw walls, bombs, and powerups
    for (int y = 0; y < game_state->height; y++) {
        for (int x = 0; x < game_state->width; x++) {
            cell_types_t cell = game_state->field[POS(x, y)];
            SpriteId spr = SPRITE_NONE;

            switch (cell) {
                case HARD_WALL:
                    spr = theme->middle;
                    if (y == 0 || game_state->field[POS(x, y - 1)] != HARD_WALL) {
                        spr = theme->top;
                    }
                    break;
                case SOFT_WALL:
                    // Randomly (deterministic based on position) choose a soft wall sprite for variety
                    spr = (x * 31 + y * 17) % 2 == 0 ? theme->broken1 : theme->broken2;
                    break;
                case BOMB:
                    spr = theme->bomb;
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

    // Draw explosions
    for (int y = 0; y < game_state->height; y++) {
        for (int x = 0; x < game_state->width; x++) {
            if (is_tile_exploding(x, y)) {
                SpriteId spr = theme->explosion.none; // TODO: choose explosion sprite based on which directions the explosion extends in
                float x_pos = x_offset + x * tile_size;
                float y_pos = y_offset + y * tile_size;

                int connection_bits = 0;
                connection_bits |= is_tile_exploding(x, y - 1) << 0; // Up
                connection_bits |= is_tile_exploding(x, y + 1) << 1; // Down
                connection_bits |= is_tile_exploding(x - 1, y) << 2; // Left
                connection_bits |= is_tile_exploding(x + 1, y) << 3; // Right

                // Invert, since the sprite names are based on which directions the explosion does NOT extend in
                connection_bits = (~connection_bits) & 0b1111;

                switch (connection_bits) {
                    case 0b0000: spr = theme->explosion.none; break;
                    case 0b0001: spr = theme->explosion.u; break;
                    case 0b0010: spr = theme->explosion.d; break;
                    case 0b0011: spr = theme->explosion.ud; break;
                    case 0b0100: spr = theme->explosion.l; break;
                    case 0b0101: spr = theme->explosion.ul; break;
                    case 0b0110: spr = theme->explosion.dl; break;
                    case 0b0111: spr = theme->explosion.udl; break;
                    case 0b1000: spr = theme->explosion.r; break;
                    case 0b1001: spr = theme->explosion.ur; break;
                    case 0b1010: spr = theme->explosion.dr; break;
                    case 0b1011: spr = theme->explosion.udr; break;
                    case 0b1100: spr = theme->explosion.lr; break;
                    case 0b1101: spr = theme->explosion.ulr; break;
                    case 0b1110: spr = theme->explosion.dlr; break;
                    case 0b1111: spr = theme->explosion.none; break;
                }

                draw_sprite(spr, x_pos, y_pos, tile_size, tile_size);
            }
        }
    }

    // Draw players on top of everything else
    for (int i = 0; i < game_state->num_players; i++) {
        const struct Player *player = &game_state->players[i];
        if (!player->alive) {
            continue;
        }

        SpriteId pid;

        switch (player->id % 8) {
            case 0: pid = SPRITE_PLAYER_1; break;
            case 1: pid = SPRITE_PLAYER_2; break;
            case 2: pid = SPRITE_PLAYER_3; break;
            case 3: pid = SPRITE_PLAYER_4; break;
            case 4: pid = SPRITE_PLAYER_5; break;
            case 5: pid = SPRITE_PLAYER_6; break;
            case 6: pid = SPRITE_PLAYER_7; break;
            case 7: pid = SPRITE_PLAYER_8; break;
        }

        float x_pos = x_offset + player->x * tile_size;
        float y_pos = y_offset + player->y * tile_size;

        // Draw player sprite
        draw_sprite(pid, x_pos, y_pos, tile_size, tile_size);
    }
}
