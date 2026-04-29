#pragma once

enum Theme {
    THEME_DIRT,
    THEME_SANDSTONE,
    THEME_GRANITE,
    THEME_STONE,
};

struct GameConfig {
    enum Theme theme;
    char previous_address[256]; // Last server address we tried to connect to
    char username[24];
};

const struct GameConfig *get_game_config(void);

void set_game_theme(enum Theme theme);
void set_previous_address(const char *address);
void set_game_username(const char *username);
void load_config(void);
