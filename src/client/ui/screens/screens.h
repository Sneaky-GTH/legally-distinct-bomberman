#define XSCREENS \
    X(main) \
    X(configure) \
    X(connecting) \
    X(game)

enum GuiScreen {
#define X(name) screen_##name,
    XSCREENS
#undef X
};

struct GuiState {
    enum GuiScreen screen;
    int mouse_x;
    int mouse_y;
};

#define X(name) void draw_##name(struct GuiState *state);
    XSCREENS
#undef X
#define X(name) void keyboard_##name(unsigned char key, int is_special);
    XSCREENS
#undef X
// #define X(name) void mouse_##name(void);
//     XSCREENS
// #undef X
