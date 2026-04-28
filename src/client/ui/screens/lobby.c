#include "./screens.h"
#include "../text.h"
#include "./connection.h"
#include "../../net/client.h"
#include "../immediate.h"
#include "../game/game.h"
#include <string.h>
#include <stdio.h>
#include <GL/glut.h>

void init_lobby(const void *data) {
    (void)data; // Unused

    enum ConnectionState state = get_connection_state();
    if (state != ESTABLISHED) {
        fprintf(stderr, "Attempted to init lobby screen while connection is not established");
        return;
    }
}

void draw_lobby() {

}

void keyboard_lobby(unsigned char key, int is_special) {
    (void)key;
    (void)is_special;
}
