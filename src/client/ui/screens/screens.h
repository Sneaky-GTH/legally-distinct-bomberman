#define XSCREENS                                                                                   \
    X(main)                                                                                        \
    X(configure)                                                                                   \
    X(connecting)                                                                                  \
    X(lobby)                                                                                       \
    X(game)

enum GuiScreen {
#define X(name) screen_##name,
    XSCREENS
#undef X
};

struct GuiState {
    int mouse_x;
    int mouse_y;
};

#define X(name) void draw_##name();
XSCREENS
#undef X
#define X(name) void keyboard_##name(unsigned char key, int is_special);
XSCREENS
#undef X
#define X(name) void init_##name(const void *data);
XSCREENS
#undef X

// Global accessor and modification functions for GUI state
struct GuiState *get_gui_state(void);
// Set the current screen and optionally pass data for initialization (e.g., server address for
// connecting screen)
void set_screen(enum GuiScreen screen, const void *data);
enum GuiScreen get_current_screen(void);

void draw_background(void);

void get_centered_offsets(int width, int height, int *out_x, int *out_y);
int draw_menu_button(const char *id, const char *text, int x, int y, int width, int height);
int draw_menu_button_colored(const char *id, const char *text, int x, int y, int width, int height, float r, float g, float b);
