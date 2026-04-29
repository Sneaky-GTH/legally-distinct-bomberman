#include "../../game/handler.h"
#include "../../game/state.h"
#include "../../net/client.h"
#include "./screens.h"
#include <GL/glut.h>
#include "../../config/config.h"
#include "../../game/state.h"
#include "../../net/client.h"
#include "../assets/themes.h"
#include "../immediate.h"
#include "../screens/screens.h"
#include "../text.h"
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define POS(x, y) ((y) * game_state->width + (x))

void draw_game_board() {
    bind_spritesheet(); // Ensure texture is bound for drawing

    const struct GameState *game_state = get_game_state();
    const struct ThemedSprites *theme = get_current_theme();

    glColor4f(1.0, 1.0, 1.0, 1.0); // Ensure full opacity and no tint

    // Calculate the size of each tile based on the window size and the map size
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);

    // Sidebar gets a fixed width
    int sidebar_width = 300;
    int board_avail_width = window_width - sidebar_width;
    
    float tile_width = (float)board_avail_width / (game_state->width + 2);
    float tile_height = (float)window_height / (game_state->height + 2);

    // Square tiles, so use the smaller of the two dimensions to avoid stretching
    float tile_size = tile_width < tile_height ? tile_width : tile_height;

    // Center the map in the board space
    float x_offset = (board_avail_width - tile_size * (game_state->width + 2)) / 2 + tile_size; // + tile_size to account for the extra row of "H" cells at the top
    float y_offset = (window_height - tile_size * (game_state->height + 2)) / 2 + tile_size; // + tile_size to account for the extra row of "H" cells at the top

    int transition_y = (int)(window_height * 0.5);

    // Draw background tiles first
    for (int y = -1; y < game_state->height + 1; y++) {
        for (int x = -1; x < game_state->width + 1; x++) {
            SpriteId spr;

            // Corners
            if (y == -1 && x == -1) {
                spr = theme->game_bg.bgTL;
            } else if (y == -1 && x == game_state->width) {
                spr = theme->game_bg.bgTR;
            } else if (y == game_state->height && x == -1) {
                spr = theme->game_bg.bgBL;
            } else if (y == game_state->height && x == game_state->width) {
                spr = theme->game_bg.bgBR;
            }

            // Top and bottom edges
            else if (y == -1) {
                spr = theme->game_bg.bgTM;
            } else if (y == game_state->height) {
                spr = theme->game_bg.bgBM;
            }

            // Left and right edges
            else if (x == -1) {
                if (y < transition_y) {
                    spr = theme->game_bg.bgHL;
                } else if (y == transition_y) {
                    spr = theme->game_bg.bgML;
                } else {
                    spr = theme->game_bg.bgLL;
                }
            } else if (x == game_state->width) {
                if (y < transition_y) {
                    spr = theme->game_bg.bgHR;
                } else if (y == transition_y) {
                    spr = theme->game_bg.bgMR;
                } else {
                    spr = theme->game_bg.bgLR;
                }
            } else {
                if (y < transition_y) {
                    spr = theme->game_bg.bgHM;
                } else if (y == transition_y) {
                    spr = theme->game_bg.bgMM;
                } else {
                    spr = theme->game_bg.bgLM;
                }
            }

            float x_pos = x_offset + x * tile_size;
            float y_pos = y_offset + y * tile_size;

            draw_sprite(spr, x_pos, y_pos, tile_size, tile_size);
        }
    }

    // Draw walls, bombs, and powerups
    for (int y = -1; y < game_state->height + 1; y++) {
        for (int x = -1; x < game_state->width + 1; x++) {
            SpriteId spr = SPRITE_NONE;

            if (x < 0 || x >= game_state->width || y < 0 || y >= game_state->height) {
                if (y == -1) {
                    spr = theme->top;
                } else if (x != -1 && x != game_state->width) {
                    spr = theme->top;
                } else {
                    spr = theme->middle; // Use top sprite for corners as well
                }
                goto draw_tile;
            }

            cell_types_t cell = game_state->field[POS(x, y)];

            switch (cell) {
                case HARD_WALL:
                    spr = theme->middle;
                    if (y != 0 || game_state->field[POS(x, y - 1)] != HARD_WALL) {
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

            draw_tile:
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
    
    // Draw sidebar
    int sb_x = board_avail_width;
    int sb_y = 0;
    blit_textbox(sb_x, sb_y, sidebar_width, window_height);
    
    glColor3f(1.0, 1.0, 1.0);
    drawTextScaled("Players", sb_x + 20, sb_y + 20, 1.5f);
    
    int cur_y = sb_y + 60;
    for (int i = 0; i < game_state->num_players; i++) {
        const struct Player *p = &game_state->players[i];
        char p_text[256];
        snprintf(p_text, sizeof(p_text), "%s%s", p->username[0] ? p->username : "Unknown", (p->id == game_state->player_id) ? " (you)" : "");
        drawText(p_text, sb_x + 20, cur_y);
        
        snprintf(p_text, sizeof(p_text), "Alive: %s", p->alive ? "Yes" : "No");
        drawText(p_text, sb_x + 180, cur_y);
        
        cur_y += 30;
    }
    
    if (draw_menu_button("disconnect-btn", "Disconnect", sb_x + 20, window_height - 60, sidebar_width - 40, 40)) {
        shutdown_network_thread();
        set_screen(screen_main, NULL);
    }
    
    if (game_state->status == 2) {
        int overlay_w = 400;
        int overlay_h = 200;
        int overlay_x = (window_width - overlay_w) / 2;
        int overlay_y = (window_height - overlay_h) / 2;
        
        // Draw a solid background for the overlay
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.0f, 0.0f, 0.0f, 0.8f); // Needs alpha blending enabled though, or just raw color
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(window_width, 0);
        glVertex2f(window_width, window_height);
        glVertex2f(0, window_height);
        glEnd();
        glEnable(GL_TEXTURE_2D);
        
        glColor4f(1.0, 1.0, 1.0, 1.0);
        blit_textbox(overlay_x, overlay_y, overlay_w, overlay_h);
        
        char win_msg[200];
        if (game_state->winner_id == 255) {
            snprintf(win_msg, sizeof(win_msg), "It's a draw!");
        } else {
            const char* winner_name = "Unknown";
            for (int i = 0; i < game_state->num_players; i++) {
                if (game_state->players[i].id == game_state->winner_id) {
                    winner_name = game_state->players[i].username;
                    break;
                }
            }
            snprintf(win_msg, sizeof(win_msg), "%s wins!", winner_name);
        }
        
        // Center text approximately
        drawTextScaled(win_msg, overlay_x + 40, overlay_y + 60, 2.0f);
        
        if (draw_menu_button("return-lobby-btn", "Return to Main Menu", overlay_x + 50, overlay_y + 130, 300, 40)) {
            shutdown_network_thread();
            set_screen(screen_main, NULL);
        }
    }
}

void draw_game() {
    if (get_connection_state() != ESTABLISHED) {
        set_screen(screen_connecting, NULL);
        return;
    }

    draw_game_board();
}

void init_game(const void *data) {}
void keyboard_game(unsigned char key, int is_special) {
    if (get_game_state()->status != 1) {
        // Don't allow any input if the game is not in progress
        return;
    }

    if ((is_special && key == GLUT_KEY_UP) || (!is_special && key == 'w')) {
        enqueue_event(&(struct GameEvent) {
            .type = EVENT_MOVE_ATTEMPT,
            .move_attempt.dir = DIR_UP,
        });
    } else if ((is_special && key == GLUT_KEY_DOWN) || (!is_special && key == 's')) {
        enqueue_event(&(struct GameEvent) {
            .type = EVENT_MOVE_ATTEMPT,
            .move_attempt.dir = DIR_DOWN,
        });
    } else if ((is_special && key == GLUT_KEY_LEFT) || (!is_special && key == 'a')) {
        enqueue_event(&(struct GameEvent) {
            .type = EVENT_MOVE_ATTEMPT,
            .move_attempt.dir = DIR_LEFT,
        });
    } else if ((is_special && key == GLUT_KEY_RIGHT) || (!is_special && key == 'd')) {
        enqueue_event(&(struct GameEvent) {
            .type = EVENT_MOVE_ATTEMPT,
            .move_attempt.dir = DIR_RIGHT,
        });
    } else if (!is_special && key == ' ') {
        enqueue_event(&(struct GameEvent) {
            .type = EVENT_PLACE_BOMB_ATTEMPT,
        });
    }
}
