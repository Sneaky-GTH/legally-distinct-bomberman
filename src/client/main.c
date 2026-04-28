#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glut.h>
#include <pthread.h>
#include "./ui/screens/screens.h"
#include "./ui/immediate.h"

#define WINDOW_WIDTH 1080
#define WINDOW_HEIGHT 720

void keyboardHandler(unsigned char key, int x, int y) {
    (void)x;
    (void)y;

    switch (get_current_screen()) {
#define X(name)                                                                                    \
    case screen_##name:                                                                            \
        keyboard_##name(key, 0);                                                                   \
        break;
        XSCREENS
#undef X
    default:
        // Unreachable
        return;
    }
}

void specialKeyboardHandler(int key, int x, int y) {
    (void)x;
    (void)y;

    switch (get_current_screen()) {
#define X(name)                                                                                    \
    case screen_##name:                                                                            \
        keyboard_##name(key, 1);                                                                   \
        break;
        XSCREENS
#undef X
    default:
        // Unreachable
        return;
    }
}

void idleLoop(void) { glutPostRedisplay(); }

void draw(void) {
    im_begin_frame();
    glClear(GL_COLOR_BUFFER_BIT);

    enum GuiScreen screen = get_current_screen();

    switch (screen) {
#define X(name)                                                                                    \
    case screen_##name:                                                                            \
        draw_##name();                                                                             \
        break;
        XSCREENS
#undef X
    default:
        // Unreachable
        return;
    }

    if (screen != get_current_screen()) {
        // Screen was changed during draw, rerender with new screen
        im_end_frame();
        draw();
        return;
    }

    glFlush();
    glutSwapBuffers();
    im_end_frame();
}

void mouseMoveHandler(int x, int y) {
    get_gui_state()->mouse_x = x;
    get_gui_state()->mouse_y = y;
}

void mouseHandler(int button, int state, int x, int y) {
    get_gui_state()->mouse_x = x;
    get_gui_state()->mouse_y = y;
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

void game_thread(); // Defined in game/game_thread.c

int main(int argc, char *argv[]) {
    const char *session = getenv("XDG_SESSION_TYPE");
    if (session == NULL || (strcmp(session, "wayland") != 0 && strcmp(session, "x11") != 0)) {
        fprintf(stderr, "Warning: XDG_SESSION_TYPE is not set to 'wayland' or 'x11'. This client "
                        "may not work properly.\n");
    }

    pthread_t game_thread_id;
    int result = pthread_create(&game_thread_id, NULL, (void* (*)(void*)) game_thread, NULL);
    if (result != 0) {
        perror("pthread_create");
        return 1;
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

    pthread_cancel(game_thread_id);
    pthread_join(game_thread_id, NULL);
    return 0;
}
