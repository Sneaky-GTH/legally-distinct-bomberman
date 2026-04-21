#include "./immediate.h"

#include <GL/glut.h>
#include <string.h>

#define MAX_FOCUS_ID_LEN 128
#define TRACKED_MOUSE_BUTTONS 3

static int mouse_down[TRACKED_MOUSE_BUTTONS] = {0};
static int queued_completed_clicks[TRACKED_MOUSE_BUTTONS] = {0};
static int frame_completed_clicks[TRACKED_MOUSE_BUTTONS] = {0};
static char focused_id[MAX_FOCUS_ID_LEN] = {0};

int is_focused(const char *component_id) {
    if (component_id == NULL || focused_id[0] == '\0') {
        return 0;
    }

    return strncmp(focused_id, component_id, MAX_FOCUS_ID_LEN) == 0;
}

void focus_component(const char *component_id) {
    if (component_id == NULL) {
        focused_id[0] = '\0';
        return;
    }

    strncpy(focused_id, component_id, MAX_FOCUS_ID_LEN - 1);
    focused_id[MAX_FOCUS_ID_LEN - 1] = '\0';
}

static ImComponentRenderFunc default_renderer_for_type(enum ImComponentType type) {
    switch (type) {
    case ImComponentInput:
    case ImComponentButton:
        return render_component_outline;
    case ImComponentUnknown:
    default:
        return render_component_outline;
    }
}

void component_init(struct ImComponent *component, const char *id, enum ImComponentType type) {
    if (component == NULL) {
        return;
    }

    component->id = id;
    component->x = 0;
    component->y = 0;
    component->width = 0;
    component->height = 0;
    component->type = type;
    component->render_func = default_renderer_for_type(type);
}

static int button_index(int mouse_button) {
    if (mouse_button == LEFT_MOUSE_BUTTON) {
        return 0;
    }
    if (mouse_button == MIDDLE_MOUSE_BUTTON) {
        return 1;
    }
    if (mouse_button == RIGHT_MOUSE_BUTTON) {
        return 2;
    }
    return -1;
}

void im_begin_frame(void) {
    for (int i = 0; i < TRACKED_MOUSE_BUTTONS; i++) {
        frame_completed_clicks[i] = queued_completed_clicks[i];
        queued_completed_clicks[i] = 0;
    }
}

void im_end_frame(void) {}

void im_mouse_button(int button, int state) {
    int index = button_index(button);
    if (index < 0) {
        return;
    }

    if (state == GLUT_DOWN) {
        mouse_down[index] = 1;
        return;
    }

    if (state == GLUT_UP) {
        if (mouse_down[index]) {
            queued_completed_clicks[index] = 1;
        }
        mouse_down[index] = 0;
    }
}

struct ImInput input_create(const char *id, char *buffer, size_t capacity) {
    struct ImInput input;

    component_init(&input.component, id, ImComponentInput);
    input.buffer = buffer;
    input.capacity = capacity;

    return input;
}

void set_component_renderer(struct ImComponent *component, ImComponentRenderFunc render_func) {
    if (component == NULL) {
        return;
    }

    component->render_func = render_func;
}

void layout_component(struct ImComponent *component, int x, int y, int width, int height) {
    if (component == NULL) {
        return;
    }

    component->x = x;
    component->y = y;
    component->width = width;
    component->height = height;
}

void render_component(struct ImComponent *component) {
    if (component == NULL) {
        return;
    }

    if (component->render_func == NULL) {
        component->render_func = default_renderer_for_type(component->type);
    }

    component->render_func(component);
}

void render_component_outline(const struct ImComponent *component) {
    if (component == NULL) {
        return;
    }

    glBegin(GL_LINE_LOOP);
    glVertex2f(component->x, component->y);
    glVertex2f(component->x + component->width, component->y);
    glVertex2f(component->x + component->width, component->y + component->height);
    glVertex2f(component->x, component->y + component->height);
    glEnd();
}

int is_mouse_over(const struct ImComponent *component, int mouse_x, int mouse_y) {
    return mouse_x >= component->x && mouse_x <= component->x + component->width &&
           mouse_y >= component->y && mouse_y <= component->y + component->height;
}

int consume_click(int mouse_button) {
    int index = button_index(mouse_button);
    if (index < 0 || frame_completed_clicks[index] == 0) {
        return 0;
    }

    frame_completed_clicks[index] = 0;
    return 1;
}
