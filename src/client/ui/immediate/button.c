#include "../game/assets/sprites.h"
#include "../immediate.h"

static void render_button(const struct ImComponent *component) {
    float size_multiplier = 2.0f; // Every button sprite is 16x16, but we want to render them at 32x32 for better visibility

    float middle_size = component->width - (12 * size_multiplier); // 6px for each side
    float height = component->height;

    // Reset color tint and bind the sprite sheet texture
    bind_spritesheet();

    // Draw left end
    draw_sprite(SPRITE_BUTTON_LEFT, component->x, component->y, 6 * size_multiplier, height);
    // Draw middle (stretch to fit)
    draw_sprite(SPRITE_BUTTON_MIDDLE, component->x + 6 * size_multiplier, component->y, middle_size, height);
    // Draw right end
    draw_sprite(SPRITE_BUTTON_RIGHT, component->x + 6 * size_multiplier + middle_size, component->y, 6 * size_multiplier, height);

    unbind_spritesheet();
}

struct ImButton button_create(const char *id) {
    struct ImButton button;

    component_init(&button.component, id, ImComponentButton);
    set_component_renderer(&button.component, render_button);

    return button;
}

int button_is_hovered(const struct ImButton *button, int mouse_x, int mouse_y) {
    if (button == NULL) {
        return 0;
    }

    return is_mouse_over(&button->component, mouse_x, mouse_y);
}

int button_clicked(const struct ImButton *button, int mouse_x, int mouse_y, int mouse_button) {
    return button_is_hovered(button, mouse_x, mouse_y) && consume_click(mouse_button);
}
