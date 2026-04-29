#include "../assets/sprites.h"
#include "../immediate.h"
#include <string.h>

static void render_input(const struct ImComponent *component) {
    float size_multiplier = 2.0f; // Every button sprite is 16x16, but we want to render them at 32x32 for better visibility

    float middle_size = component->width - (14 * size_multiplier);

    float height = component->height;

    // Reset color tint and bind the sprite sheet texture
    bind_spritesheet();

    // Draw left end
    draw_sprite(SPRITE_INPUT_LEFT, component->x, component->y, 6 * size_multiplier, height);
    // Draw middle (stretch to fit)
    draw_sprite(SPRITE_INPUT_MIDDLE, component->x + 6 * size_multiplier, component->y, middle_size, height);
    // Draw right end
    draw_sprite(SPRITE_INPUT_RIGHT, component->x + 6 * size_multiplier + middle_size, component->y, 8 * size_multiplier, height);

    unbind_spritesheet();
}

struct ImInput input_create(const char *id, char *buffer, size_t capacity) {
    struct ImInput input;

    component_init(&input.component, id, ImComponentInput);
    input.buffer = buffer;
    input.capacity = capacity;

    set_component_renderer(&input.component, render_input);

    return input;
}

void input_focus(struct ImInput *input) {
    if (input == NULL || input->component.id == NULL) {
        return;
    }

    focus_component(input->component.id);
}

void input_blur(struct ImInput *input) {
    if (input == NULL || input->component.id == NULL) {
        focus_component(NULL);
        return;
    }

    if (is_focused(input->component.id)) {
        focus_component(NULL);
    }
}

int input_is_focused(const struct ImInput *input) {
    if (input == NULL || input->component.id == NULL) {
        return 0;
    }

    return is_focused(input->component.id);
}

void input_handle_key(struct ImInput *input, unsigned char key) {
    if (input == NULL || input->buffer == NULL || input->capacity == 0) {
        return;
    }

    size_t len = strlen(input->buffer);

    if (key == '\b') {
        if (len > 0) {
            input->buffer[len - 1] = '\0';
        }
        return;
    }

    if (key < 32 || key > 126 || len + 1 >= input->capacity) {
        return;
    }

    input->buffer[len] = (char)key;
    input->buffer[len + 1] = '\0';
}
