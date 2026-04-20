#include "./text.h"

void drawText(const char* text, GLfloat x, GLfloat y) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

int textWidth(void* font, const char* s, size_t count) {
    int w = 0;
    for (size_t i = 0; i < count && s[i] != '\0'; ++i) {
        w += glutBitmapWidth(font, (unsigned char)s[i]);
    }
    return w;
}
