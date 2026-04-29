#include "./text.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHARS 256

typedef struct {
    int id;
    int x, y, width, height;
    int xoffset, yoffset, xadvance;
} FontChar;

static FontChar font_chars[MAX_CHARS];
static GLuint font_texture;
static int font_tex_width = 123;
static int font_tex_height = 158;
static int font_line_height = 18;
static int font_base = 13;

static unsigned char* load_rgba(const char* rgba_filename, int width, int height) {
    FILE* file = fopen(rgba_filename, "rb");
    if (!file) return NULL;
    
    unsigned char* pixels = (unsigned char*)malloc(width * height * 4);
    if (!pixels) {
        fclose(file);
        return NULL;
    }
    
    size_t result = fread(pixels, 1, width * height * 4, file);
    if (result != (size_t)(width * height * 4)) {
        free(pixels);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    return pixels;
}

void initText(void) {
    memset(font_chars, 0, sizeof(font_chars));
    
    FILE *f = fopen("assets/font.txt", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "common ", 7) == 0) {
                sscanf(line, "common lineHeight=%d base=%d scaleW=%d scaleH=%d", 
                       &font_line_height, &font_base, &font_tex_width, &font_tex_height);
            } else if (strncmp(line, "char ", 5) == 0) {
                int id, x, y, width, height, xoffset, yoffset, xadvance;
                if (sscanf(line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
                           &id, &x, &y, &width, &height, &xoffset, &yoffset, &xadvance) == 8) {
                    if (id >= 0 && id < MAX_CHARS) {
                        font_chars[id].id = id;
                        font_chars[id].x = x;
                        font_chars[id].y = y;
                        font_chars[id].width = width;
                        font_chars[id].height = height;
                        font_chars[id].xoffset = xoffset;
                        font_chars[id].yoffset = yoffset;
                        font_chars[id].xadvance = xadvance;
                    }
                }
            }
        }
        fclose(f);
    } else {
        fprintf(stderr, "Error: Could not load assets/font.txt\n");
    }

    unsigned char* pixels = load_rgba("assets/font.rgba", font_tex_width, font_tex_height);
    if (pixels) {
        glGenTextures(1, &font_texture);
        glBindTexture(GL_TEXTURE_2D, font_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font_tex_width, font_tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        free(pixels);
    } else {
        fprintf(stderr, "Error: Could not load assets/font.rgba\n");
    }
}

void drawText(const char *text, GLfloat x, GLfloat y) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font_texture);
    
    // Enable blending for transparent background
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBegin(GL_QUADS);
    
    GLfloat current_x = x;
    GLfloat current_y = y;
    
    for (const char *c = text; *c != '\0'; c++) {
        unsigned char char_idx = (unsigned char)*c;
        FontChar fc = font_chars[char_idx];
        
        if (fc.width > 0 && fc.height > 0) {
            GLfloat draw_x = current_x + fc.xoffset;
            GLfloat draw_y = current_y + fc.yoffset - font_base; // Adjust y based on baseline
            
            GLfloat tx = (GLfloat)fc.x / font_tex_width;
            GLfloat ty = (GLfloat)fc.y / font_tex_height;
            GLfloat tw = (GLfloat)fc.width / font_tex_width;
            GLfloat th = (GLfloat)fc.height / font_tex_height;
            
            glTexCoord2f(tx, ty);           glVertex2f(draw_x, draw_y);
            glTexCoord2f(tx + tw, ty);      glVertex2f(draw_x + fc.width, draw_y);
            glTexCoord2f(tx + tw, ty + th); glVertex2f(draw_x + fc.width, draw_y + fc.height);
            glTexCoord2f(tx, ty + th);      glVertex2f(draw_x, draw_y + fc.height);
        }
        
        current_x += fc.xadvance;
    }
    
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawTextScaled(const char *text, GLfloat x, GLfloat y, GLfloat scale) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1.0f);
    drawText(text, 0, 0);
    glPopMatrix();
}

int textWidth(const char *s, size_t count) {
    int w = 0;
    for (size_t i = 0; i < count && s[i] != '\0'; ++i) {
        unsigned char char_idx = (unsigned char)s[i];
        w += font_chars[char_idx].xadvance;
    }
    return w;
}
