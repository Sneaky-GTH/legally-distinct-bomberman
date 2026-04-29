#include "../../config/config.h"
#include "../assets/sprites.h"
#include "../immediate.h"
#include "../text.h"
#include "./screens.h"
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>

void draw_configure() {
    int total_w = 400;
    int total_h = 420;
    int cx, cy;
    get_centered_offsets(total_w, total_h, &cx, &cy);
    
    glColor3f(1.0, 1.0, 1.0);
    
    const char *title = "Configure";
    int title_w = textWidth(title, strlen(title)) * 2;
    int title_cx = cx + total_w / 2;
    
    blit_textbox(title_cx - title_w/2 - 20, cy, title_w + 40, 60);
    drawTextScaled(title, title_cx - title_w/2, cy + 32, 2.0f);

    int box_y = cy + 80;
    int box_w = 240;
    int box_h = 230;
    
    blit_textbox(title_cx - box_w/2, box_y, box_w, box_h);
    drawText("Theme:", title_cx - box_w/2 + 20, box_y + 20);

    const int btn_width = box_w - 40;
    const int btn_height = 32;
    int cur_y = box_y + 50;
    
    struct {
        enum Theme theme;
        const char *name;
    } themes[] = {
        { THEME_DIRT, "Dirt" },
        { THEME_SANDSTONE, "Sandstone" },
        { THEME_GRANITE, "Granite" },
        { THEME_STONE, "Stone" },
    };
    
    const struct GameConfig *cfg = get_game_config();
    
    for (int i = 0; i < 4; i++) {
        float r = -1.0f, g = -1.0f, b = -1.0f;
        if (cfg->theme == themes[i].theme) {
            r = 0.5f; g = 1.0f; b = 0.5f;
        }
        
        char btn_id[32];
        snprintf(btn_id, sizeof(btn_id), "theme-%d", i);
        
        if (draw_menu_button_colored(btn_id, themes[i].name, title_cx - btn_width/2, cur_y, btn_width, btn_height, r, g, b)) {
            set_game_theme(themes[i].theme);
        }
        
        cur_y += btn_height + 10;
    }
    
    int back_y = box_y + box_h + 20;
    if (draw_menu_button("back-btn", "Back", title_cx - box_w/2, back_y, box_w, btn_height)) {
        set_screen(screen_main, NULL);
    }
}

void init_configure(const void *data) {}
void keyboard_configure(unsigned char key, int is_special) {}
