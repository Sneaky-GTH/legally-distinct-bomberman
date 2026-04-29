#include "../../config/config.h"
#include "../immediate.h"
#include "../text.h"
#include "./screens.h"
#include <GL/glut.h>
#include <string.h>

void draw_configure() {
    struct GuiState *state = get_gui_state();
    
    glColor3f(1.0, 1.0, 1.0);
    drawText("Configuration", 40, 40);
    drawText("Theme:", 40, 80);

    const int btn_width = 150;
    const int btn_height = 32;
    int cur_y = 100;
    
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
        struct ImButton btn = button_create(themes[i].name);
        layout_component(&btn.component, 40, cur_y, btn_width, btn_height);
        
        if (cfg->theme == themes[i].theme) {
            glColor3f(0.5, 1.0, 0.5);
        } else if (button_is_hovered(&btn, state->mouse_x, state->mouse_y)) {
            glColor3f(0.7, 0.7, 0.7);
        } else {
            glColor3f(1.0, 1.0, 1.0);
        }
        
        render_component(&btn.component);
        glColor3f(1.0, 1.0, 1.0);
        drawText(themes[i].name, 61, cur_y + 18);
        
        if (button_clicked(&btn, state->mouse_x, state->mouse_y, LEFT_MOUSE_BUTTON)) {
            set_game_theme(themes[i].theme);
        }
        
        cur_y += btn_height + 10;
    }
    
    cur_y += 20;
    
    struct ImButton back_btn = button_create("back-btn");
    layout_component(&back_btn.component, 40, cur_y, 80, btn_height);
    
    if (button_is_hovered(&back_btn, state->mouse_x, state->mouse_y)) {
        glColor3f(0.7, 0.7, 0.7);
    } else {
        glColor3f(1.0, 1.0, 1.0);
    }
    render_component(&back_btn.component);

    glColor3f(1.0, 1.0, 1.0);
    drawText("Back", 61, cur_y + 18);
    
    if (button_clicked(&back_btn, state->mouse_x, state->mouse_y, LEFT_MOUSE_BUTTON)) {
        set_screen(screen_main, NULL);
    }
}

void init_configure(const void *data) {}
void keyboard_configure(unsigned char key, int is_special) {}
