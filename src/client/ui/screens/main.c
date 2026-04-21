#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glut.h>
#include "../text.h"
#include "../immediate.h"
#include "./screens.h"
#include "./connection.h"

struct MainState {
    int is_address_selected;
    char server_address[256];
    char server_address_error[256];
};

static struct MainState MAIN_STATE = {
    .is_address_selected =
        1, // Enable by default (TODO: Remove this when input handling is implemented)
    .server_address = ""};

static struct ImInput server_address_input(void) {
    return input_create("input-id-1", MAIN_STATE.server_address, sizeof(MAIN_STATE.server_address));
}

int validate_server_address(const char *address) {
    if (strlen(address) == 0) {
        strcpy(MAIN_STATE.server_address_error, "Server address cannot be empty.");
        return 0;
    }

    // TODO: Add more validation (e.g., regex for IP:port or hostname:port)

    MAIN_STATE.server_address_error[0] = '\0';
    return 1;
}

void keyboard_main(unsigned char key, int is_special) {
    if (is_special) {
        return;
    }

    struct ImInput input = server_address_input();
    if (input_is_focused(&input)) {
        input_handle_key(&input, key);
        MAIN_STATE.server_address_error[0] = '\0'; // Clear error on input
        return;
    }
}

void draw_main(struct GuiState *state) {
    int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);

    // Set text color to white
    glColor3f(1.0, 1.0, 1.0);
    drawText("Legally Distinct Bomberman", 40, 40);
    drawText("Server address:", 40, 80);

    const int INPUT_WIDTH = 400;
    const int INPUT_HEIGHT = 32;
    const int INPUT_X = 40;
    const int INPUT_Y = 100;

    struct ImInput input = server_address_input();

    layout_component(&input.component, INPUT_X, INPUT_Y, INPUT_WIDTH, INPUT_HEIGHT);
    if (MAIN_STATE.server_address_error[0] != '\0') {
        glColor3f(1.0, 0.5, 0.5); // Error color
    }
    render_component(&input.component);
    glColor3f(1.0, 1.0, 1.0);

    if (MAIN_STATE.server_address_error[0] != '\0') {
        drawText(MAIN_STATE.server_address_error, INPUT_X, INPUT_Y + INPUT_HEIGHT + 20);
    }

    if (is_mouse_over(&input.component, state->mouse_x, state->mouse_y) &&
        consume_click(LEFT_MOUSE_BUTTON)) {
        input_focus(&input);
    }

    if (MAIN_STATE.server_address[0] == '\0' && !input_is_focused(&input)) {
        drawText("Enter server address...", INPUT_X + 10, INPUT_Y + 23);
    } else {
        if (timeSinceStart % 1000 < 500 && input_is_focused(&input)) {
            glColor3f(1.0, 1.0, 1.0); // Cursor color
            size_t len = strlen(MAIN_STATE.server_address);
            int caretX =
                INPUT_X + 10 + textWidth(GLUT_BITMAP_HELVETICA_18, MAIN_STATE.server_address, len);
            glBegin(GL_LINES);
            glVertex2f(caretX, INPUT_Y + 5);
            glVertex2f(caretX, INPUT_Y + INPUT_HEIGHT - 5);
            glEnd();
        }
        drawText(MAIN_STATE.server_address, INPUT_X + 10, INPUT_Y + 23);
    }

    // Draw connect button
    const int BUTTON_WIDTH = 100;
    const int BUTTON_HEIGHT = 32;
    const int BUTTON_X = INPUT_X + INPUT_WIDTH + 20;
    const int BUTTON_Y = INPUT_Y;

    struct ImButton connect_button = button_create("button-connect");
    layout_component(&connect_button.component, BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT);

    if (button_is_hovered(&connect_button, state->mouse_x, state->mouse_y)) {
        glColor3f(0.7, 0.7, 0.7); // Hover color
    } else {
        glColor3f(1.0, 1.0, 1.0); // Normal color
    }

    render_component(&connect_button.component);

    drawText("Connect", BUTTON_X + 10, BUTTON_Y + 23);

    if (button_clicked(&connect_button, state->mouse_x, state->mouse_y, LEFT_MOUSE_BUTTON)) {
        input_blur(&input);
        if (validate_server_address(MAIN_STATE.server_address)) {
            state->screen = screen_connecting;
            set_address(MAIN_STATE.server_address);
        }
    }

    // Has the user clicked outside the input field?
    if (consume_click(LEFT_MOUSE_BUTTON) && !is_mouse_over(&input.component, state->mouse_x, state->mouse_y)) {
        input_blur(&input);
    }
}
