#include "../../game/state.h"
#include "./game.h"
#include <GL/glut.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define HARD_WALL_COLOR 0.3f, 0.3f, 0.3f
#define SOFT_WALL_COLOR 0.4f, 0.7f, 0.4f
#define EMPTY_COLOR 0.0f, 0.0f, 0.0f
#define BOMB_COLOR 0.8f, 0.8f, 0.0f
#define EXPLOSION_COLOR 1.0f, 0.5f, 0.0f
#define BONUS_COLOR 0.0f, 0.8f, 0.8f

#define POS(x, y) ((y) * game_state->width + (x))

void draw_game_board() {
    // TODO: use textures

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

    for (int y = 0; y < game_state->height; y++) {
        for (int x = 0; x < game_state->width; x++) {
            cell_types_t cell = game_state->field[POS(x, y)];
            switch (cell) {
                case HARD_WALL:
                    glColor3f(HARD_WALL_COLOR);
                    break;
                case SOFT_WALL:
                    glColor3f(SOFT_WALL_COLOR);
                    break;
                case BOMB:
                    glColor3f(BOMB_COLOR);
                    break;
                case EXPLOSION:
                    glColor3f(EXPLOSION_COLOR);
                    break;
                case POWERUP_SPEED:
                case POWERUP_SIZE:
                case POWERUP_TIME:
                case POWERUP_COUNT:
                    glColor3f(BONUS_COLOR);
                    break;
                    
                case EMPTY:
                default:
                    glColor3f(EMPTY_COLOR);
                    break;
            }

            float x_pos = x_offset + x * tile_size;
            float y_pos = y_offset + y * tile_size;

            glBegin(GL_QUADS);
            glVertex2f(x_pos, y_pos);
            glVertex2f(x_pos + tile_size, y_pos);
            glVertex2f(x_pos + tile_size, y_pos + tile_size);
            glVertex2f(x_pos, y_pos + tile_size);
            glEnd();
        }
    }

    for (int i = 0; i < game_state->num_players; i++) {
        const struct Player *player = &game_state->players[i];
        if (!player->alive) {
            continue;
        }

        // Draw players as circles for now, with different colors based on player ID
        float hue = (player->id * 0.61803398875f); // Golden ratio conjugate to distribute hues evenly
        float r = 0.5f + 0.5f * cosf(2 * M_PI * hue);
        float g = 0.5f + 0.5f * cosf(2 * M_PI * hue + 2 * M_PI / 3);
        float b = 0.5f + 0.5f * cosf(2 * M_PI * hue + 4 * M_PI / 3);
        glColor3f(r, g, b);

        float x_pos = x_offset + player->x * tile_size + tile_size / 2;
        float y_pos = y_offset + player->y * tile_size + tile_size / 2;
        float radius = tile_size * 0.4f;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x_pos, y_pos);
        for (int angle = 0; angle <= 360; angle += 30) {
            float rad = angle * M_PI / 180.0f;
            glVertex2f(x_pos + cosf(rad) * radius, y_pos + sinf(rad) * radius);
        }
        glEnd();   
    }
}
