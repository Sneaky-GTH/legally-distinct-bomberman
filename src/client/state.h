enum GuiScreen {
    Main,
    Configure,
    Game,
};

struct GuiState {
    enum GuiScreen screen;
    int mouse_x;
    int mouse_y;
};

struct MainState {
    char* server_address;
};

static struct GuiState GUI_STATE = {
    .screen = Main,
    .mouse_x = 0,
    .mouse_y = 0,
};
static struct MainState MAIN_STATE = { .server_address = NULL };