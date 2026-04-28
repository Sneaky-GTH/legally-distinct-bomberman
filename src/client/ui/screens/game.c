#include "../../game/handler.h"
#include "../../game/state.h"
#include "../../net/client.h"
#include "../game/game.h"
#include "./screens.h"
#include <GL/glut.h>

void draw_game() {
    if (get_connection_state() != ESTABLISHED) {
        set_screen(screen_connecting, NULL);
        return;
    }

    draw_game_board();
}

void init_game(const void *data) {}
void keyboard_game(unsigned char key, int is_special) {
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
