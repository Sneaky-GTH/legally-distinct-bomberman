#include "../assets/themes.h"
#include "../assets/sprites.h"
#include "../immediate.h"
#include "../text.h"
#include "./screens.h"
#include <GL/glut.h>
#include <string.h>

static enum GuiScreen current_screen = screen_main;
static struct GuiState GUI_STATE = {
    .mouse_x = 0,
    .mouse_y = 0,
};

struct GuiState *get_gui_state(void) { return &GUI_STATE; }

enum GuiScreen get_current_screen(void) { return current_screen; }

void set_screen(enum GuiScreen screen, const void *data) {
    current_screen = screen;

    switch (screen) {
#define X(name)                                                                                    \
    case screen_##name:                                                                            \
        init_##name(data);                                                                         \
        break;
        XSCREENS
#undef X
    default:
        // Unreachable
        return;
    }
}

void draw_background(void) {
    const struct ThemedSprites *theme = get_current_theme();

    float window_height = glutGet(GLUT_WINDOW_HEIGHT);
    float window_width = glutGet(GLUT_WINDOW_WIDTH);

    float tile_size = 64.0f; // Background tiles are 64x64
    float width_in_tiles = window_width / tile_size;
    float height_in_tiles = window_height / tile_size;

    // There are 7 tiles for the background, in a gradient.
    // Every second tile is a gradient tile that should be used when transitioning between two background colors.
    // 3 gradient tiles, 4 solid color tiles
    float change_background_every = height_in_tiles / 7.0f;

    int bg_tile = 1; // 1 - 7

    bind_spritesheet();
    glColor4f(1.0, 1.0, 1.0, 1.0); // Ensure full brightness and no tint

    // Draw background first
    for (int y = 0; y < height_in_tiles; y++) {
        if (y >= change_background_every * bg_tile && bg_tile < 7) {
            bg_tile = (y / change_background_every) + 1;
        }

        SpriteId bg_sprite;

        switch (bg_tile) {
            case 1: bg_sprite = theme->background1; break;
            case 2: bg_sprite = theme->background12; break;
            case 3: bg_sprite = theme->background2; break;
            case 4: bg_sprite = theme->background23; break;
            case 5: bg_sprite = theme->background3; break;
            case 6: bg_sprite = theme->background34; break;
            case 7: bg_sprite = theme->background4; break;
            default: bg_sprite = theme->background1; break; // UNREACHABLE
        }

        for (int x = 0; x < width_in_tiles; x++) {
            draw_sprite(bg_sprite, x * tile_size, y * tile_size, tile_size, tile_size);
        }

        if (bg_tile % 2 == 0) {
            // This is a gradient tile, so do not draw two of them in a row
            bg_tile++;
        }
    }

    unbind_spritesheet();
}

void get_centered_offsets(int width, int height, int *out_x, int *out_y) {
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);
    *out_x = (window_width - width) / 2;
    *out_y = (window_height - height) / 2;
}

int draw_menu_button_colored(const char *id, const char *text, int x, int y, int width, int height, float r, float g, float b) {
    struct GuiState *state = get_gui_state();
    struct ImButton btn = button_create(id);
    layout_component(&btn.component, x, y, width, height);
    
    if (r >= 0.0f) {
        glColor3f(r, g, b);
    } else if (button_is_hovered(&btn, state->mouse_x, state->mouse_y)) {
        glColor3f(0.7f, 0.7f, 0.7f);
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    
    render_component(&btn.component);
    
    glColor3f(1.0f, 1.0f, 1.0f);
    size_t len = strlen(text);
    int t_width = textWidth(text, len);
    
    drawText(text, x + (width - t_width) / 2, y + height / 2 + 2);
    
    return button_clicked(&btn, state->mouse_x, state->mouse_y, LEFT_MOUSE_BUTTON);
}

int draw_menu_button(const char *id, const char *text, int x, int y, int width, int height) {
    return draw_menu_button_colored(id, text, x, y, width, height, -1.0f, -1.0f, -1.0f);
}

