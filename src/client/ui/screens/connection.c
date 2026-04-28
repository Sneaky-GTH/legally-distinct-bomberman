#include "./screens.h"
#include "../text.h"
#include "./connection.h"
#include "../../net/client.h"
#include "../immediate.h"
#include <string.h>
#include <stdio.h>
#include <GL/glut.h>

void init_connecting(const void *data) {
    enum ConnectionState state = get_connection_state();
    if (state != DISCONNECTED) {
        fprintf(stderr, "Attempted to init connection screen while connection is active");
        return;
    }

    struct ConnectScreenData conn_data = *(struct ConnectScreenData*) data; 

    spawn_network_thread(conn_data.server_address);
}

void draw_connecting() {
    enum ConnectionState connection_state = get_connection_state();
    struct GuiState *state = get_gui_state();

    // Set text color to white
    glColor3f(1.0, 1.0, 1.0);
    drawText("Legally Distinct Bomberman", 40, 40);

    drawText("Connecting to ", 40, 80);
    drawText(get_server_address(), 162, 80);

    switch (connection_state) {
        case CONNECTING:
            drawText("Connecting...", 40, 120);
            break;
        case HANDSHAKE:
            drawText("Performing handshake...", 40, 120);
            break;
        case DISCONNECTED:
            drawText("An error occurred. View the console for more information.", 40, 120);
            break;
        case TEARDOWN:
            drawText("Shutting down connection...", 40, 120);
            break;
        case ESTABLISHED:
            set_screen(screen_lobby, NULL);
            return;
    }

    drawText(get_status_message(), 40, 160);

    // Draw back button
    // if (connection_state == DISCONNECTED) {

        const int BUTTON_WIDTH = 80;
        const int BUTTON_HEIGHT = 32;
        const int BUTTON_X = 40;
        const int BUTTON_Y = 180;

        struct ImButton back_button = button_create("button-connection-back");
        layout_component(&back_button.component, BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT);

        if (button_is_hovered(&back_button, state->mouse_x, state->mouse_y)) {
            glColor3f(0.7, 0.7, 0.7); // Hover color
        } else {
            glColor3f(1.0, 1.0, 1.0); // Normal color
        }

        render_component(&back_button.component);

        drawText("Back", BUTTON_X + 10, BUTTON_Y + 23);

        if (button_clicked(&back_button, state->mouse_x, state->mouse_y, LEFT_MOUSE_BUTTON)) {
            shutdown_network_thread();
            set_screen(screen_main, NULL);
        }
    // }
}

void keyboard_connecting(unsigned char key, int is_special) {
    (void)key;
    (void)is_special;
}
