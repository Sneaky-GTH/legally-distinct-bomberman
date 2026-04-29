#include "../../net/client.h"
#include "../../game/state.h"
#include "../../game/handler.h"
#include "../assets/sprites.h"
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

    int total_w = 400;
    int total_h = 360;
    int cx, cy;
    get_centered_offsets(total_w, total_h, &cx, &cy);
    
    glColor3f(1.0, 1.0, 1.0);
    
    const char *title = "Legally Distinct Bomberman";
    int title_w = textWidth(title, strlen(title)) * 2;
    int title_cx = cx + total_w / 2;
    
    blit_textbox(title_cx - title_w/2 - 20, cy, title_w + 40, 60);
    drawTextScaled(title, title_cx - title_w/2, cy + 32, 2.0f);

    int box_y = cy + 80;
    int box_w = 400;
    int box_h = 240;

    box_h = 40 + game_state->num_players * 40;

    blit_textbox(title_cx - box_w/2, box_y, box_w, box_h);
    
    int content_x = title_cx - box_w/2 + 20;

    if (game_state->status == 0) {
        // Lobby
        drawText("Waiting for players...", content_x, box_y + 20);

        // List players
        for (int i = 0; i < game_state->num_players; i++) {
            const struct Player *player = &game_state->players[i];
            char player_text[256];
            int is_self = player->id == game_state->player_id;
            snprintf(player_text, sizeof(player_text), "%s%s", player->username, is_self ? " (you)" : "");
            
            int p_y = box_y + 60 + i * 40;
            drawText(player_text, content_x, p_y);
            snprintf(player_text, sizeof(player_text), "Ready: %s", player->ready ? "Yes" : "No");
            drawText(player_text, content_x + 180, p_y);

            if (is_self && !player->ready) {
                const int BUTTON_WIDTH = 84;
                const int BUTTON_HEIGHT = 32;
                
                if (draw_menu_button("button-ready-toggle", "Ready", content_x + 270, p_y - 24, BUTTON_WIDTH, BUTTON_HEIGHT)) {
                    enqueue_event(&(struct GameEvent) { .type = EVENT_SELF_READY });
                }
            }
        }
    } else if (game_state->status == 1) {
        set_screen(screen_game, NULL);
    } else if (game_state->status == 2) {
        drawText("Game already over!", content_x, box_y + 20);
    }
}

void keyboard_lobby(unsigned char key, int is_special) {
    (void)key;
    (void)is_special;
}
