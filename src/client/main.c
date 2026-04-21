#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glut.h>
#include "./state.h"
#include "./ui/text.h"
#include "./ui/immediate.h"

#define WINDOW_WIDTH 1080
#define WINDOW_HEIGHT 720

static struct ImInput server_address_input(void) {
    return input_create("input-id-1", MAIN_STATE.server_address, sizeof(MAIN_STATE.server_address));
}

void keyboardHandler(unsigned char key, int x, int y) {
    (void)x;
    (void)y;

    if (GUI_STATE.screen != Main) {
        return;
    }

    struct ImInput input = server_address_input();
    if (input_is_focused(&input)) {
        input_handle_key(&input, key);
    }
}

void idleLoop(void) { glutPostRedisplay(); }

void drawMainScreen(void) {
    int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);

    // Set text color to white
    glColor3f(1.0, 1.0, 1.0);
    drawText("Legally Distinct Bomberman", 40, 40);
    drawText("Server address:", 40, 80);

    const int INPUT_WIDTH = 400;
    const int INPUT_HEIGHT = 32;
    const int INPUT_X = 40;
    const int INPUT_Y = 100;

    struct ImInput input = server_address_input();

    layout_component(&input.component, INPUT_X, INPUT_Y, INPUT_WIDTH, INPUT_HEIGHT);
    render_component(&input.component);

    if (is_mouse_over(&input.component, GUI_STATE.mouse_x, GUI_STATE.mouse_y) &&
        consume_click(LEFT_MOUSE_BUTTON)) {
        input_focus(&input);
    }

    if (MAIN_STATE.server_address[0] == '\0' && !input_is_focused(&input)) {
        drawText("Enter server address...", INPUT_X + 10, INPUT_Y + 23);
    } else {
        if (timeSinceStart % 1000 < 500 && input_is_focused(&input)) {
            glColor3f(1.0, 1.0, 1.0); // Cursor color
            size_t len = strlen(MAIN_STATE.server_address);
            int caretX =
                INPUT_X + 10 + textWidth(GLUT_BITMAP_HELVETICA_18, MAIN_STATE.server_address, len);

            glBegin(GL_LINES);
            glVertex2f(caretX, INPUT_Y + 5);
            glVertex2f(caretX, INPUT_Y + INPUT_HEIGHT - 5);
            glEnd();
        }
        drawText(MAIN_STATE.server_address, INPUT_X + 10, INPUT_Y + 23);
    }

    // Draw connect button
    const int BUTTON_WIDTH = 100;
    const int BUTTON_HEIGHT = 32;
    const int BUTTON_X = INPUT_X + INPUT_WIDTH + 20;
    const int BUTTON_Y = INPUT_Y;

    struct ImButton connect_button = button_create("button-connect");
    layout_component(&connect_button.component, BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT);

    if (button_is_hovered(&connect_button, GUI_STATE.mouse_x, GUI_STATE.mouse_y)) {
        glColor3f(0.7, 0.7, 0.7); // Hover color
    } else {
        glColor3f(1.0, 1.0, 1.0); // Normal color
    }

    render_component(&connect_button.component);

    drawText("Connect", BUTTON_X + 10, BUTTON_Y + 23);

    if (button_clicked(&connect_button, GUI_STATE.mouse_x, GUI_STATE.mouse_y, LEFT_MOUSE_BUTTON)) {
        input_blur(&input);
        GUI_STATE.screen = Configure;
    }
}

void drawConfigure(void) {}

void drawGame(void) {}

void draw(void) {
    im_begin_frame();
    glClear(GL_COLOR_BUFFER_BIT);

    switch (GUI_STATE.screen) {
    case Main:
        drawMainScreen();
        break;
    case Configure:
        drawConfigure();
        break;
    case Game:
        drawGame();
        break;
    default:
        // Unreachable
    }

    glFlush();
    glutSwapBuffers();
    im_end_frame();
}

void specialKeyboardHandler(int key, int x, int y) {
    (void)key;
    (void)x;
    (void)y;
}

void mouseMoveHandler(int x, int y) {
    GUI_STATE.mouse_x = x;
    GUI_STATE.mouse_y = y;
}

void mouseHandler(int button, int state, int x, int y) {
    GUI_STATE.mouse_x = x;
    GUI_STATE.mouse_y = y;
    im_mouse_button(button, state);
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char *argv[]) {
    const char *session = getenv("XDG_SESSION_TYPE");
    if (session == NULL || (strcmp(session, "wayland") != 0 && strcmp(session, "x11") != 0)) {
        fprintf(stderr, "Warning: XDG_SESSION_TYPE is not set to 'wayland' or 'x11'. This client "
                        "may not work properly.\n");
    }

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Legally Distinct Bomberman");
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glColor3f(0.0, 0.0, 0.0);
    glPointSize(1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // gluOrtho2D(-620.0, 620.0, -340.0, 340.0);

    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardHandler);
    glutSpecialFunc(specialKeyboardHandler);
    glutMouseFunc(mouseHandler);
    glutPassiveMotionFunc(mouseMoveHandler);
    glutMotionFunc(mouseMoveHandler);
    glutIdleFunc(idleLoop);
    glutDisplayFunc(draw);
    glutMainLoop();

    return 0;
}
