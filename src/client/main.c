#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glut.h>
#include "./state.h"
#include "./ui/text.h"

#define WINDOW_WIDTH 1080
#define WINDOW_HEIGHT 720

void keyboardHandler(unsigned char key, int x, int y) {
    if (GUI_STATE.screen == Main) {
        if (key == '\r') { // Enter key
            if (MAIN_STATE.server_address != NULL) {
                free(MAIN_STATE.server_address);
            }
            MAIN_STATE.server_address = NULL;
        } else if (key == '\b') { // Backspace key
            if (MAIN_STATE.server_address != NULL) {
                size_t len = strlen(MAIN_STATE.server_address);
                if (len > 0) {
                    MAIN_STATE.server_address[len - 1] = '\0';
                }
            }
        } else if (key >= 32 && key <= 126) { // Printable characters
            size_t len = MAIN_STATE.server_address == NULL ? 0 : strlen(MAIN_STATE.server_address);
            char* new_address = realloc(MAIN_STATE.server_address, len + 2); // +1 for new char, +1 for null terminator
            if (new_address == NULL) {
                fprintf(stderr, "Error: Failed to allocate memory for server address.\n");
                return;
            }
            MAIN_STATE.server_address = new_address;
            MAIN_STATE.server_address[len] = key;
            MAIN_STATE.server_address[len + 1] = '\0';
        }
    }
}

void idleLoop(void) {
    glutPostRedisplay();
}

void drawMainScreen(void) {
    // Set text color to white
    glColor3f(1.0, 1.0, 1.0);
    drawText("Legally Distinct Bomberman", 40, 40);
    drawText("Server address:", 40, 80);
    
    const int INPUT_WIDTH = 400;
    const int INPUT_HEIGHT = 32;
    const int INPUT_X = 40;
    const int INPUT_Y = 100;

    // Draw input box
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(INPUT_X, INPUT_Y);
    glVertex2f(INPUT_X + INPUT_WIDTH, INPUT_Y);
    glVertex2f(INPUT_X + INPUT_WIDTH, INPUT_Y + INPUT_HEIGHT);
    glVertex2f(INPUT_X, INPUT_Y + INPUT_HEIGHT);
    glEnd();

    if (MAIN_STATE.server_address == NULL) {
        drawText("Enter server address...", INPUT_X + 10, INPUT_Y + 24);
    } else {
        drawText(MAIN_STATE.server_address, INPUT_X + 10, INPUT_Y + 24);
    }
}

void drawConfigure(void) {

}

void drawGame(void) {

}

void draw(void) {
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
}

void specialKeyboardHandler(int key, int x, int y) {

}

void mouseHandler(int button, int state, int x, int y) {
    GUI_STATE.mouse_x = x;
    GUI_STATE.mouse_y = y;
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char* argv[]) {
    const char* session = getenv("XDG_SESSION_TYPE");
    if (session == NULL || (strcmp(session, "wayland") != 0 && strcmp(session, "x11") != 0)) {
        fprintf(stderr, "Warning: XDG_SESSION_TYPE is not set to 'wayland' or 'x11'. This client may not work properly.\n");
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
    glutIdleFunc(idleLoop);
    glutDisplayFunc(draw);
    glutMainLoop();
    
    return 0;
}
