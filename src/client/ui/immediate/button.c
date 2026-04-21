#include "../immediate.h"

struct ImButton button_create(const char *id) {
    struct ImButton button;

    component_init(&button.component, id, ImComponentButton);

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
