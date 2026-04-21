#include <string.h>
#include "../immediate.h"

void input_focus(struct ImInput* input) {
    if (input == NULL || input->component.id == NULL) {
        return;
    }

    focus_component(input->component.id);
}

void input_blur(struct ImInput* input) {
    if (input == NULL || input->component.id == NULL) {
        focus_component(NULL);
        return;
    }

    if (is_focused(input->component.id)) {
        focus_component(NULL);
    }
}

int input_is_focused(const struct ImInput* input) {
    if (input == NULL || input->component.id == NULL) {
        return 0;
    }

    return is_focused(input->component.id);
}

void input_handle_key(struct ImInput* input, unsigned char key) {
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

    input->buffer[len] = (char) key;
    input->buffer[len + 1] = '\0';
}
