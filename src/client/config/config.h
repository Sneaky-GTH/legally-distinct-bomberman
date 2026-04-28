#pragma once

enum Theme {
    THEME_DIRT,
    THEME_SANDSTONE,
    THEME_GRANITE,
    THEME_STONE,
};

struct GameConfig {
    enum Theme theme;
};

const struct GameConfig *get_game_config(void);

void set_game_theme(enum Theme theme);
void load_config(void);
