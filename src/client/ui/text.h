#pragma once
#include <GL/glut.h>
#include <stddef.h>
#include <stdint.h>

void initText(void);
void drawText(const char *text, GLfloat x, GLfloat y);
void drawTextScaled(const char *text, GLfloat x, GLfloat y, GLfloat scale);
int textWidth(const char *s, size_t count);
