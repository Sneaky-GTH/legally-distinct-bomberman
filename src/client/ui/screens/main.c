#include "../../config/config.h"
#include "../assets/sprites.h"
#include "../immediate.h"
#include "../text.h"
#include "./connection.h"
#include "./screens.h"
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct MainState {
    int is_address_selected;
    char username[24];
    char server_address[256];
    char server_address_error[256];
};

static struct MainState MAIN_STATE = {
    .is_address_selected = 0,
    .username = "",
    .server_address = "",
};

static struct ImInput server_address_input(void) {
    return input_create("input-id-1", MAIN_STATE.server_address, sizeof(MAIN_STATE.server_address));
}

static struct ImInput server_username_input(void) {
    return input_create("input-id-user", MAIN_STATE.username, sizeof(MAIN_STATE.username));
}

void init_main(const void *data) {
    (void)data; // Unused
    const char* previous_address = get_game_config()->previous_address;
    if (previous_address[0] != '\0') {
        strncpy(MAIN_STATE.server_address, previous_address, sizeof(MAIN_STATE.server_address) - 1);
        MAIN_STATE.server_address[sizeof(MAIN_STATE.server_address) - 1] = '\0';
    }
    const char* username = get_game_config()->username;
    if (username[0] != '\0') {
        strncpy(MAIN_STATE.username, username, sizeof(MAIN_STATE.username) - 1);
        MAIN_STATE.username[sizeof(MAIN_STATE.username) - 1] = '\0';
    }
}

void keyboard_main(unsigned char key, int is_special) {
    if (is_special) {
        return;
    }

    struct ImInput input_addr = server_address_input();
    if (input_is_focused(&input_addr)) {
        input_handle_key(&input_addr, key);
        MAIN_STATE.server_address_error[0] = '\0'; // Clear error on input
        return;
    }

    struct ImInput input_user = server_username_input();
    if (input_is_focused(&input_user)) {
        input_handle_key(&input_user, key);
        return;
    }
}

void draw_main() {
    struct GuiState *state = get_gui_state();
    int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);

    glColor3f(1.0, 1.0, 1.0);

    int total_w = 520;
    int total_h = 360;
    int cx, cy;
    get_centered_offsets(total_w, total_h, &cx, &cy);

    // "Legally Distinct Bomberman"
    const char *title = "Legally Distinct Bomberman";
    int title_w = textWidth(title, strlen(title)) * 2;
    int title_cx = cx + total_w / 2;
    blit_textbox(title_cx - title_w/2 - 20, cy, title_w + 40, 60);
    drawTextScaled(title, title_cx - title_w/2, cy + 32, 2.0f);

    int inputs_y = cy + 100;
    blit_textbox(title_cx - 240, inputs_y, 480, 150);

    const int INPUT_WIDTH = 300;
    const int INPUT_HEIGHT = 32;
    int input_x = title_cx - 150 + 20;

    // Username input
    drawText("Username:", title_cx - 220, inputs_y + 43);
    struct ImInput input_user = server_username_input();
    layout_component(&input_user.component, input_x, inputs_y + 25, INPUT_WIDTH, INPUT_HEIGHT);
    render_component(&input_user.component);

    if (MAIN_STATE.username[0] == '\0' && !input_is_focused(&input_user)) {
        drawText("Enter username...", input_x + 14, inputs_y + 44);
    } else {
        if (timeSinceStart % 1000 < 500 && input_is_focused(&input_user)) {
            glColor3f(0.0, 0.0, 0.0);
            size_t len = strlen(MAIN_STATE.username);
            int caretX = input_x + 14 + textWidth(MAIN_STATE.username, len);
            glBegin(GL_LINES);
            glVertex2f(caretX, inputs_y + 34);
            glVertex2f(caretX, inputs_y + 25 + INPUT_HEIGHT - 9);
            glEnd();
            glColor3f(1.0, 1.0, 1.0);
        }
        drawText(MAIN_STATE.username, input_x + 14, inputs_y + 44);
    }
    if (is_mouse_over(&input_user.component, state->mouse_x, state->mouse_y) && consume_click(LEFT_MOUSE_BUTTON)) {
        input_focus(&input_user);
    }

    // Address input
    drawText("Address:", title_cx - 220, inputs_y + 103);
    struct ImInput input_addr = server_address_input();
    layout_component(&input_addr.component, input_x, inputs_y + 85, INPUT_WIDTH, INPUT_HEIGHT);
    render_component(&input_addr.component);

    if (MAIN_STATE.server_address_error[0] != '\0') {
        glColor3f(1.0, 0.5, 0.5);
        drawText(MAIN_STATE.server_address_error, input_x, inputs_y + 135);
        glColor3f(1.0, 1.0, 1.0);
    }

    if (MAIN_STATE.server_address[0] == '\0' && !input_is_focused(&input_addr)) {
        drawText("Enter server address...", input_x + 14, inputs_y + 103);
    } else {
        if (timeSinceStart % 1000 < 500 && input_is_focused(&input_addr)) {
            glColor3f(0.0, 0.0, 0.0);
            size_t len = strlen(MAIN_STATE.server_address);
            int caretX = input_x + 14 + textWidth(MAIN_STATE.server_address, len);
            glBegin(GL_LINES);
            glVertex2f(caretX, inputs_y + 94);
            glVertex2f(caretX, inputs_y + 85 + INPUT_HEIGHT - 9);
            glEnd();
            glColor3f(1.0, 1.0, 1.0);
        }
        drawText(MAIN_STATE.server_address, input_x + 14, inputs_y + 103);
    }
    if (is_mouse_over(&input_addr.component, state->mouse_x, state->mouse_y) && consume_click(LEFT_MOUSE_BUTTON)) {
        input_focus(&input_addr);
    }

    // Configure and Connect buttons
    int buttons_y = inputs_y + 170;
    if (draw_menu_button("button-config", "Configure", title_cx - 150, buttons_y, 140, 32)) {
        input_blur(&input_user);
        input_blur(&input_addr);
        set_screen(screen_configure, NULL);
        return;
    }

    if (draw_menu_button("button-connect", "Connect", title_cx + 10, buttons_y, 140, 32)) {
        if (strlen(MAIN_STATE.username) == 0) {
            // Cannot connect without username
            input_blur(&input_addr);
            input_focus(&input_user);
        } else {
            input_blur(&input_user);
            input_blur(&input_addr);
            struct ConnectScreenData data = {
                .server_address = "",
                .username = "",
            };
            strncpy(data.server_address, MAIN_STATE.server_address, sizeof(data.server_address) - 1);
            strncpy(data.username, MAIN_STATE.username, sizeof(data.username) - 1);
            set_previous_address(data.server_address);
            set_game_username(MAIN_STATE.username);
            set_screen(screen_connecting, &data);
            return;
        }
    }

    if (consume_click(LEFT_MOUSE_BUTTON) &&
        !is_mouse_over(&input_user.component, state->mouse_x, state->mouse_y) &&
        !is_mouse_over(&input_addr.component, state->mouse_x, state->mouse_y)) {
        input_blur(&input_user);
        input_blur(&input_addr);
    }
}
