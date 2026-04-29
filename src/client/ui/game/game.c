#include "../../config/config.h"
#include "../../game/state.h"
#include "./assets/sprites.h"
#include "./game.h"
#include <GL/glut.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define POS(x, y) ((y) * game_state->width + (x))

struct ThemedExplosionSprites {
    SpriteId ulr;
    SpriteId udl;
    SpriteId ud;
    SpriteId udr;
    SpriteId none;
    SpriteId lr;
    SpriteId dr;
    SpriteId dl;
    SpriteId d;
    SpriteId u;
    SpriteId dlr;
    SpriteId ur;
    SpriteId ul;
    SpriteId l;
    SpriteId r;
};

struct ThemedSprites {
    SpriteId top;
    SpriteId middle;
    SpriteId broken1;
    SpriteId broken2;
    SpriteId bomb;
    SpriteId background1;
    SpriteId background12;
    SpriteId background2;
    SpriteId background23;
    SpriteId background3;
    SpriteId background34;
    SpriteId background4;
    struct ThemedExplosionSprites explosion;
};

static const struct ThemedSprites THEMES[] = {
    {
        .top = SPRITE_DIRT_TOP,
        .middle = SPRITE_DIRT,
        .broken1 = SPRITE_BROKEN_DIRT_1,
        .broken2 = SPRITE_BROKEN_DIRT_2,
        .bomb = SPRITE_BOMB,
        .background1 = SPRITE_BG_BLUE_1,
        .background12 = SPRITE_BG_BLUE_12,
        .background2 = SPRITE_BG_BLUE_2,
        .background23 = SPRITE_BG_BLUE_23,
        .background3 = SPRITE_BG_BLUE_3,
        .background34 = SPRITE_BG_BLUE_34,
        .background4 = SPRITE_BG_BLUE_4,
        .explosion = {
            .ulr = SPRITE_EXPLOSION_ULR,
            .udl = SPRITE_EXPLOSION_UDL,
            .ud = SPRITE_EXPLOSION_UD,
            .udr = SPRITE_EXPLOSION_UDR,
            .none = SPRITE_EXPLOSION,
            .lr = SPRITE_EXPLOSION_LR,
            .dr = SPRITE_EXPLOSION_DR,
            .dl = SPRITE_EXPLOSION_DL,
            .d = SPRITE_EXPLOSION_D,
            .u = SPRITE_EXPLOSION_U,
            .dlr = SPRITE_EXPLOSION_DLR,
            .ur = SPRITE_EXPLOSION_UR,
            .ul = SPRITE_EXPLOSION_UL,
            .l = SPRITE_EXPLOSION_L,
            .r = SPRITE_EXPLOSION_R,
        },
    },
    {
        .top = SPRITE_SANDSTONE_TOP,
        .middle = SPRITE_SANDSTONE,
        .broken1 = SPRITE_BROKEN_SANDSTONE_1,
        .broken2 = SPRITE_BROKEN_SANDSTONE_2,
        .bomb = SPRITE_BOMB_ALT,
        .background1 = SPRITE_BG_YELLOW_1,
        .background12 = SPRITE_BG_YELLOW_12,
        .background2 = SPRITE_BG_YELLOW_2,
        .background23 = SPRITE_BG_YELLOW_23,
        .background3 = SPRITE_BG_YELLOW_3,
        .background34 = SPRITE_BG_YELLOW_34,
        .background4 = SPRITE_BG_YELLOW_4,
        .explosion = {
            .ulr = SPRITE_EXPLOSION_ALT_ULR,
            .udl = SPRITE_EXPLOSION_ALT_UDL,
            .ud = SPRITE_EXPLOSION_ALT_UD,
            .udr = SPRITE_EXPLOSION_ALT_UDR,
            .none = SPRITE_EXPLOSION_ALT,
            .lr = SPRITE_EXPLOSION_ALT_LR,
            .dr = SPRITE_EXPLOSION_ALT_DR,
            .dl = SPRITE_EXPLOSION_ALT_DL,
            .d = SPRITE_EXPLOSION_ALT_D,
            .u = SPRITE_EXPLOSION_ALT_U,
            .dlr = SPRITE_EXPLOSION_ALT_DLR,
            .ur = SPRITE_EXPLOSION_ALT_UR,
            .ul = SPRITE_EXPLOSION_ALT_UL,
            .l = SPRITE_EXPLOSION_ALT_L,
            .r = SPRITE_EXPLOSION_ALT_R,
        },
    },
    {
        .top = SPRITE_GRANITE_TOP,
        .middle = SPRITE_GRANITE,
        .broken1 = SPRITE_BROKEN_GRANITE_1,
        .broken2 = SPRITE_BROKEN_GRANITE_2,
        .bomb = SPRITE_BOMB_ALT,
        .background1 = SPRITE_BG_PURPLE_1,
        .background12 = SPRITE_BG_PURPLE_12,
        .background2 = SPRITE_BG_PURPLE_2,
        .background23 = SPRITE_BG_PURPLE_23,
        .background3 = SPRITE_BG_PURPLE_3,
        .background34 = SPRITE_BG_PURPLE_34,
        .background4 = SPRITE_BG_PURPLE_4,
        .explosion = {
            .ulr = SPRITE_EXPLOSION_ALT_ULR,
            .udl = SPRITE_EXPLOSION_ALT_UDL,
            .ud = SPRITE_EXPLOSION_ALT_UD,
            .udr = SPRITE_EXPLOSION_ALT_UDR,
            .none = SPRITE_EXPLOSION_ALT,
            .lr = SPRITE_EXPLOSION_ALT_LR,
            .dr = SPRITE_EXPLOSION_ALT_DR,
            .dl = SPRITE_EXPLOSION_ALT_DL,
            .d = SPRITE_EXPLOSION_ALT_D,
            .u = SPRITE_EXPLOSION_ALT_U,
            .dlr = SPRITE_EXPLOSION_ALT_DLR,
            .ur = SPRITE_EXPLOSION_ALT_UR,
            .ul = SPRITE_EXPLOSION_ALT_UL,
            .l = SPRITE_EXPLOSION_ALT_L,
            .r = SPRITE_EXPLOSION_ALT_R,
        },
    },
    {
        .top = SPRITE_STONE_TOP,
        .middle = SPRITE_STONE,
        .broken1 = SPRITE_BROKEN_STONE_1,
        .broken2 = SPRITE_BROKEN_STONE_2,
        .bomb = SPRITE_BOMB,
        .background1 = SPRITE_BG_GRAY_1,
        .background12 = SPRITE_BG_GRAY_12,
        .background2 = SPRITE_BG_GRAY_2,
        .background23 = SPRITE_BG_GRAY_23,
        .background3 = SPRITE_BG_GRAY_3,
        .background34 = SPRITE_BG_GRAY_34,
        .background4 = SPRITE_BG_GRAY_4,
        .explosion = {
            .ulr = SPRITE_EXPLOSION_ULR,
            .udl = SPRITE_EXPLOSION_UDL,
            .ud = SPRITE_EXPLOSION_UD,
            .udr = SPRITE_EXPLOSION_UDR,
            .none = SPRITE_EXPLOSION,
            .lr = SPRITE_EXPLOSION_LR,
            .dr = SPRITE_EXPLOSION_DR,
            .dl = SPRITE_EXPLOSION_DL,
            .d = SPRITE_EXPLOSION_D,
            .u = SPRITE_EXPLOSION_U,
            .dlr = SPRITE_EXPLOSION_DLR,
            .ur = SPRITE_EXPLOSION_UR,
            .ul = SPRITE_EXPLOSION_UL,
            .l = SPRITE_EXPLOSION_L,
            .r = SPRITE_EXPLOSION_R,
        },
    },
};

void draw_game_board() {
    bind_spritesheet(); // Ensure texture is bound for drawing

    const struct GameState *game_state = get_game_state();
    const struct ThemedSprites *theme;
    switch (get_game_config()->theme) {
        case THEME_DIRT: theme = &THEMES[0]; break;
        case THEME_SANDSTONE: theme = &THEMES[1]; break;
        case THEME_GRANITE: theme = &THEMES[2]; break;
        case THEME_STONE: theme = &THEMES[3]; break;
        default: theme = &THEMES[0]; break; // UNREACHABLE
    }

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
            case 1: bg_sprite = theme->background1; break;
            case 2: bg_sprite = theme->background12; break;
            case 3: bg_sprite = theme->background2; break;
            case 4: bg_sprite = theme->background23; break;
            case 5: bg_sprite = theme->background3; break;
            case 6: bg_sprite = theme->background34; break;
            case 7: bg_sprite = theme->background4; break;
            default: bg_sprite = theme->background1; break; // UNREACHABLE
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
