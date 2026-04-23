#include "./screens.h"
#include "../text.h"
#include "./connection.h"
#include <string.h>
#include <GL/glut.h>

struct ConnectionState {
    char server_address[256];
    char status[256];
};

static struct ConnectionState CONNECTION_STATE = {
    .server_address = "",
    .status = "Connecting...",
};

void init_connecting(const void *data) {
    const struct ConnectScreenData *connect_data = (const struct ConnectScreenData *)data;
    strncpy(CONNECTION_STATE.server_address, connect_data->server_address,
            sizeof(CONNECTION_STATE.server_address) - 1);
    CONNECTION_STATE.server_address[sizeof(CONNECTION_STATE.server_address) - 1] = '\0';
}

void draw_connecting() {

    // Set text color to white
    glColor3f(1.0, 1.0, 1.0);
    drawText("Legally Distinct Bomberman", 40, 40);

    drawText("Connecting to ", 40, 80);
    drawText(CONNECTION_STATE.server_address, 162, 80);

    drawText(CONNECTION_STATE.status, 40, 120);
}

void keyboard_connecting(unsigned char key, int is_special) {
    (void)key;
    (void)is_special;
}
