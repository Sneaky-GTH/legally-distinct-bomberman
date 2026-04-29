#ifndef TEXT_H
#define TEXT_H

#include <GL/glut.h>
#include <stddef.h>
#include <stdint.h>

void initText(void);
void drawText(const char *text, GLfloat x, GLfloat y);
int textWidth(void *font, const char *s, size_t count);

#endif // TEXT_H
