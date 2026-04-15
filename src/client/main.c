#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct playingField {
    int width;
    int height;
    char** cell;
};

int init_playingField(struct playingField* field, int w, int h) {
    field->width = w;
    field->height = h;

    char** c = (char**)malloc(h * sizeof(char*));
    for (int i = 0; i < h; i++) {
        c[i] = (char*)malloc(w * sizeof(char));
    }

    field->cell = c;

    for (int i = 0; i < field->height; i++) {
        for (int j = 0; j < field->width; j++) {
            field->cell[i][j] = 'a';
        }
    }

    return 1;
}

void print_playingField(struct playingField* field) {
    for (int i = 0; i < field->height; i++) {
        for (int j = 0; j < field->width; j++) {
            printf("%c ", field->cell[i][j]);
        }
        printf("\n");
    }
}


void free_playingField(struct playingField* field) {
    for (int i = 0; i < field->height; i++) {
        free(field->cell[i]);
    }

    free(field->cell);
    free(field);
}

int main(void) {

    struct playingField* field = (struct playingField*)malloc(sizeof(struct playingField));
    init_playingField(field, 7, 5);
    print_playingField(field);

    free_playingField(field);

    return 0;
}
