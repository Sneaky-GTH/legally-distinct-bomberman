#include "../../net/client.h"
#include "../assets/sprites.h"
#include "../immediate.h"
#include "../text.h"
#include "./connection.h"
#include "./screens.h"
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>

void init_connecting(const void *data) {
    if (data != NULL) {
        enum ConnectionState state = get_connection_state();
        if (state != DISCONNECTED) {
            fprintf(stderr, "Attempted to init connection screen while connection is active");
            return;
        }

        struct ConnectScreenData conn_data = *(struct ConnectScreenData*) data; 
        spawn_network_thread(conn_data.server_address, conn_data.username);
    }
}

void draw_connecting() {
    enum ConnectionState connection_state = get_connection_state();
    
    int total_w = 400;
    int total_h = 320;
    int cx, cy;
    get_centered_offsets(total_w, total_h, &cx, &cy);

    glColor3f(1.0, 1.0, 1.0);
    
    const char *title = "Legally Distinct Bomberman";
    int title_w = textWidth(title, strlen(title)) * 2;
    int title_cx = cx + total_w / 2;
    
    blit_textbox(title_cx - title_w/2 - 20, cy, title_w + 40, 60);
    drawTextScaled(title, title_cx - title_w/2, cy + 32, 2.0f);

    int box_y = cy + 80;
    int box_w = 320;
    int box_h = 100;
    blit_textbox(title_cx - box_w/2, box_y, box_w, box_h);

    int text_y = box_y + 30;

    switch (connection_state) {
        case CONNECTING:
            drawText("Connecting...", title_cx - box_w/2 + 20, text_y);
            break;
        case HANDSHAKE:
            drawText("Performing handshake...", title_cx - box_w/2 + 20, text_y);
            break;
        case DISCONNECTED:
            drawText("Disconnected.", title_cx - box_w/2 + 20, text_y);
            break;
        case TEARDOWN:
            drawText("Shutting down connection...", title_cx - box_w/2 + 20, text_y);
            break;
        case ESTABLISHED:
            set_screen(screen_lobby, NULL);
            return;
    }

    drawText(get_status_message(), title_cx - box_w/2 + 20, text_y + 40);

    int btn_height = 32;
    int back_y = box_y + box_h + 20;

    if (draw_menu_button("button-connection-back", "Back", title_cx - box_w/2, back_y, box_w, btn_height)) {
        shutdown_network_thread();
        set_screen(screen_main, NULL);
    }
}

void keyboard_connecting(unsigned char key, int is_special) {
    (void)key;
    (void)is_special;
}
