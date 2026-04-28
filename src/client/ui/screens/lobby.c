#include "../../net/client.h"
#include "../../game/state.h"
#include "../../game/handler.h"
#include "../game/game.h"
#include "../immediate.h"
#include "../text.h"
#include "./connection.h"
#include "./screens.h"
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>

void init_lobby(const void *data) {
    (void)data; // Unused

    enum ConnectionState state = get_connection_state();
    if (state != ESTABLISHED) {
        fprintf(stderr, "Attempted to init lobby screen while connection is not established");
        return;
    }
}

void draw_lobby() {
    const struct GameState *game_state = get_game_state();
    if (get_connection_state() != ESTABLISHED) {
        set_screen(screen_connecting, NULL);
        return;
    }

    if (game_state->status == 0) {
        // Lobby
        glColor3f(1.0, 1.0, 1.0);
        drawText("Waiting for players...", 40, 40);

        // List players
        for (int i = 0; i < game_state->num_players; i++) {
            const struct Player *player = &game_state->players[i];
            char player_text[256];
            int is_self = player->id == game_state->player_id;
            snprintf(player_text, sizeof(player_text), "Player %d%s", player->id, is_self ? " (you)" : "");
            drawText(player_text, 40, 80 + i * 40);
            snprintf(player_text, sizeof(player_text), "Ready: %s", player->ready ? "Yes" : "No");
            drawText(player_text, 200, 80 + i * 40);

            if (is_self && !player->ready) {
                // Draw button to toggle ready state
                const int BUTTON_WIDTH = 80;
                const int BUTTON_HEIGHT = 32;
                const int BUTTON_X = 300;
                const int BUTTON_Y = 80 + i * 40 - 20;
                struct ImButton ready_button = button_create("button-ready-toggle");
                layout_component(&ready_button.component, BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT);

                if (button_is_hovered(&ready_button, get_gui_state()->mouse_x, get_gui_state()->mouse_y)) {
                    glColor3f(0.7, 0.7, 0.7); // Hover color
                } else {
                    glColor3f(1.0, 1.0, 1.0); // Normal color
                }

                render_component(&ready_button.component);
                drawText("Ready", BUTTON_X + 10, BUTTON_Y + 23);
    
                if (button_clicked(&ready_button, get_gui_state()->mouse_x, get_gui_state()->mouse_y, LEFT_MOUSE_BUTTON) && is_self) {
                    enqueue_event(&(struct GameEvent) { .type = EVENT_SELF_READY });

                }
            }
        }
    } else if (game_state->status == 1) {
        set_screen(screen_game, NULL);
    } else if (game_state->status == 2) {
        glColor3f(1.0, 1.0, 1.0);
        drawText("Game already over!", 40, 40);
    }
}

void keyboard_lobby(unsigned char key, int is_special) {
    (void)key;
    (void)is_special;
}
