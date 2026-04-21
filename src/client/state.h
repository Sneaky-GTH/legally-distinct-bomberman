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
    int is_address_selected;
    char server_address[256];
};

static struct GuiState GUI_STATE = {
    .screen = Main,
    .mouse_x = 0,
    .mouse_y = 0,
};
static struct MainState MAIN_STATE = {
    .is_address_selected = 1, // Enable by default (TODO: Remove this when input handling is implemented)
    .server_address = ""
};
