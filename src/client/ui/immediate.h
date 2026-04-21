#ifndef UI_IMMEDIATE_H
#define UI_IMMEDIATE_H

#include <stddef.h>

#define LEFT_MOUSE_BUTTON 0
#define MIDDLE_MOUSE_BUTTON 1
#define RIGHT_MOUSE_BUTTON 2

struct ImComponent;
typedef void (*ImComponentRenderFunc)(const struct ImComponent* component);

enum ImComponentType {
    ImComponentUnknown,
    ImComponentInput,
    ImComponentButton,
};

struct ImComponent {
    const char* id;
    int x;
    int y;
    int width;
    int height;
    enum ImComponentType type;
    ImComponentRenderFunc render_func;
};

struct ImInput {
    struct ImComponent component;
    char* buffer;
    size_t capacity;
};

struct ImButton {
    struct ImComponent component;
};

int is_focused(const char* component_id);
void focus_component(const char* component_id);

void im_begin_frame(void);
void im_end_frame(void);
void im_mouse_button(int button, int state);

void component_init(struct ImComponent* component, const char* id, enum ImComponentType type);
void set_component_renderer(struct ImComponent* component, ImComponentRenderFunc render_func);
void layout_component(struct ImComponent* component, int x, int y, int width, int height);
void render_component(struct ImComponent* component);
void render_component_outline(const struct ImComponent* component);
int consume_click(int mouse_button);
int is_mouse_over(const struct ImComponent* component, int mouse_x, int mouse_y);

struct ImButton button_create(const char* id);
int button_is_hovered(const struct ImButton* button, int mouse_x, int mouse_y);
int button_clicked(const struct ImButton* button, int mouse_x, int mouse_y, int mouse_button);

struct ImInput input_create(const char* id, char* buffer, size_t capacity);
void input_focus(struct ImInput* input);
void input_blur(struct ImInput* input);
int input_is_focused(const struct ImInput* input);
void input_handle_key(struct ImInput* input, unsigned char key);

#endif
