#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include "server/logic/playingfield.h"

int init_playingField(PlayingField* field, uint16_t w, uint16_t h) {
    field->width = w;
    field->height = h;

    uint16_t* c = malloc(h * w * sizeof(uint16_t));

    field->cell = c;

    for (int i = 0; i < field->height; i++) {
        for (int j = 0; j < field->width; j++) {
            CELL(field, j, i) = (uint8_t)'.';
        }
    }

    return 1;
}

void print_playingField(PlayingField* field) {
    for (int i = 0; i < field->height; i++) {
        for (int j = 0; j < field->width; j++) {
            printf("%c ", CELL(field, j, i));
        }
        printf("\n");
    }
}


void free_playingField(PlayingField* field) {
    free(field->cell);
    free(field);
}

void prepare_playingField(PlayingField *field) {
    // Open "./assets/fields/*", pick randomly
    DIR *d;
    struct dirent *dir;
    int count = 0;
    d = opendir("./assets/fields");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strstr(dir->d_name, ".txt")) count++;
        }
        closedir(d);
    }
    
    if (count == 0) {
        fprintf(stderr, "No fields found\n");
        return;
    }

    srand(time(NULL));
    int r = rand() % count;
    
    char filepath[256];
    d = opendir("./assets/fields");
    if (d) {
        int i = 0;
        while ((dir = readdir(d)) != NULL) {
            if (strstr(dir->d_name, ".txt")) {
                if (i == r) {
                    snprintf(filepath, sizeof(filepath), "./assets/fields/%s", dir->d_name);
                    break;
                }
                i++;
            }
        }
        closedir(d);
    }

    FILE *f = fopen(filepath, "r");

    if (!f) {
        fprintf(stderr, "Failed to open field config\n");
        return;
    }

    uint16_t w, h;
    if (fscanf(f, "%hu %hu\n", &h, &w) != 2) {
        fprintf(stderr, "Failed to read width and height from field config\n");
        fclose(f);
        return;
    }

    if (w != field->width || h != field->height) {
        field->cell = realloc(field->cell, w * h * sizeof(uint16_t));
        field->width = w;
        field->height = h;
    }

    // Read the rest of the lines to fill the field with the corresponding characters
    int c;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            c = fgetc(f);
            if (c == EOF) {
                fprintf(stderr, "Failed to read cell content from field config\n");
                fclose(f);
                return;
            }
            CELL(field, j, i) = (uint8_t)c;
        }
        if ((c = fgetc(f)) != '\n' && c != EOF) {
            fprintf(stderr, "Expected newline at the end of line in field config, got %c\n", c);
            fclose(f);
            return;
        }
    }

    fclose(f);
}


uint8_t SAFE_GET_CELL(PlayingField* field, uint16_t x, uint16_t y) {
    if (x < 0 || y < 0) {
        return 'f';
    }

    if (x > field->width - 1 || y > field->height - 1) {
        return 'f';
    }

    return CELL(field, x, y);
}

int SAFE_SET_CELL(PlayingField* field, uint16_t x, uint16_t y, uint16_t v) {
    if (x < 0 || y < 0) {
        return -1;
    }

    if (x > field->width - 1 || y > field->height - 1) {
        return -1;
    }

    CELL(field, x, y) = v;
    return 0;
}


int move_cell_contents(PlayingField* field, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    CELL(field, x2, y2) = CELL(field, x1, y1);
    CELL(field, x1, y1) = '.';

    return 0;
}


uint16_t cell_to_uint(PlayingField* field, uint16_t x, uint16_t y) {
    return (y) * (field)->width + (x);
}



