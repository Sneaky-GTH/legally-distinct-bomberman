#include "./config.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

static pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct GameConfig CONFIG = {
    .theme = THEME_DIRT,
    .previous_address = "",
    .username = "Player",
};

const struct GameConfig *get_game_config(void) {
    return &CONFIG;
}

static const char* get_home_directory() {
    const char* home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "Warning: HOME environment variable not set, using current directory for config file\n");
        return ".";
    }
    return home;
}

static const char* get_config_directory() {
    static char path[512];
    snprintf(path, sizeof(path), "%s/.config/legally_distinct_bomberman", get_home_directory());
    return path;
}

static const char* get_config_file_path() {
    static char path[512];
    snprintf(path, sizeof(path), "%s/config.bin", get_config_directory());
    return path;
}

static void ensure_config_directory_exists() {
    const char* config_dir = get_config_directory();
    if (access(config_dir, F_OK) == -1) {
        if (mkdir(config_dir, 0755) != 0) {
            perror("Failed to create config directory");
        }
    }
}

void load_config() {
    pthread_mutex_lock(&file_mutex);
    if (access(get_config_file_path(), F_OK) == -1) {
        // Config file doesn't exist, use defaults
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    ensure_config_directory_exists();
    int config_fd = open(get_config_file_path(), O_RDONLY);
    if (config_fd == -1) {
        perror("Failed to open config file");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    struct GameConfig loaded_config;
    ssize_t bytes_read = read(config_fd, &loaded_config, sizeof(loaded_config));
    if (bytes_read == sizeof(loaded_config)) {
        CONFIG = loaded_config;
    } else if (bytes_read == -1) {
        perror("Failed to read config file");
    } else {
        fprintf(stderr, "Config file has unexpected size, using defaults\n");
    }
    close(config_fd);
    pthread_mutex_unlock(&file_mutex);
}

static void save_config() {
    pthread_mutex_lock(&file_mutex);
    ensure_config_directory_exists();
    int config_fd = open(get_config_file_path(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (config_fd == -1) {
        perror("Failed to open config file for writing");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    ssize_t bytes_written = write(config_fd, &CONFIG, sizeof(CONFIG));
    if (bytes_written != sizeof(CONFIG)) {
        perror("Failed to write config file");
    }
    close(config_fd);
    pthread_mutex_unlock(&file_mutex);
}

static void async_save_config() {
    pthread_create(&(pthread_t){0}, NULL, (void*(*)(void*)) save_config, NULL);
}

void set_game_theme(enum Theme theme) {
    CONFIG.theme = theme;
    async_save_config();
}

void set_previous_address(const char* address) {
    strncpy(CONFIG.previous_address, address, 255);
    CONFIG.previous_address[255] = '\0';
    async_save_config();
}

void set_game_username(const char* username) {
    strncpy(CONFIG.username, username, 23);
    CONFIG.username[23] = '\0';
    async_save_config();
}
