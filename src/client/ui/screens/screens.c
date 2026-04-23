#include "./screens.h"


static enum GuiScreen current_screen = screen_main;
static struct GuiState GUI_STATE = {
    .mouse_x = 0,
    .mouse_y = 0,
};

struct GuiState *get_gui_state(void) {
    return &GUI_STATE;
}

enum GuiScreen get_current_screen(void) {
    return current_screen;
}

void set_screen(enum GuiScreen screen, const void *data) {
    current_screen = screen;

    switch (screen) {
#define X(name) case screen_##name: init_##name(data); break;
        XSCREENS
#undef X
    default:
        // Unreachable
        return;
    }
}
