#pragma once

#include <stdint.h>
#include "./map.h"

#ifndef bool
#define bool uint8_t
#define true 1
#define false 0
#endif

typedef enum { GAME_LOBBY = 0, GAME_RUNNING = 1, GAME_END = 2 } game_status_t;

typedef enum { DIR_UP = 0, DIR_DOWN = 1, DIR_LEFT = 2, DIR_RIGHT = 3 } direction_t;

typedef enum { BONUS_NONE = 0, BONUS_SPEED = 1, BONUS_RADIUS = 2, BONUS_TIMER = 3 } bonus_type_t;

typedef enum {
    MSG_HELLO = 0,
    MSG_WELCOME = 1,
    MSG_DISCONNECT = 2,
    MSG_PING = 3,
    MSG_PONG = 4,
    MSG_LEAVE = 5,
    MSG_ERROR = 6,
    MSG_MAP = 7,
    MSG_SET_READY = 10,
    MSG_SET_STATUS = 20,
    MSG_WINNER = 23,
    MSG_MOVE_ATTEMPT = 30,
    MSG_BOMB_ATTEMPT = 31,
    MSG_MOVED = 40,
    MSG_BOMB = 41,
    MSG_EXPLOSION_START = 42,
    MSG_EXPLOSION_END = 43,
    MSG_DEATH = 44,
    MSG_BONUS_AVAILABLE = 45,
    MSG_BONUS_RETRIEVED = 46,
    MSG_BLOCK_DESTROYED = 47
} msg_type_t;

typedef struct {
    char client_id[20];
    char client_name[30];
} data_hello_t;

typedef struct {
    uint8_t client_id;
    bool is_ready;
    char client_name[30];
} clients_t;

typedef struct {
    char server_name[20];
    game_status_t status;
    uint8_t len;
    clients_t *clients;
} data_welcome_t;

typedef struct {
} data_leave_t;

typedef struct {
} data_disconnect_t;

typedef struct {
} data_ping_t;

typedef struct {
} data_pong_t;

typedef struct {
    char *error_message;
} data_error_t;

typedef struct {
} data_set_ready_t;

typedef struct {
    uint8_t status;
} data_set_status_t;

typedef struct {
    uint8_t winner_id;
} data_winner_t;

typedef struct {
    uint8_t height;
    uint8_t width;
    cell_types_t *map; // cell_types_t maps to char
} data_map_t;

typedef struct {
    direction_t direction;
} data_move_attempt_t;

typedef struct {
    uint8_t player_id;
    uint16_t new_position;
} data_moved_t;

typedef struct {
    uint16_t position;
} data_bomb_attempt_t;

typedef struct {
    uint8_t player_id;
    uint16_t position;
} data_bomb_t;

typedef struct {
    uint8_t radius;
    uint16_t position;
} data_explosion_t;

typedef struct {
    uint8_t player_id;
} data_death_t;

typedef struct {
    bonus_type_t bonus_type;
    uint16_t position;
} data_bonus_available_t;

typedef struct {
    uint8_t player_id;
    uint16_t position;
} data_bonus_retrieved_t;

typedef struct {
    uint16_t position;
} data_block_destroyed_t;

typedef struct Message {
    msg_type_t type;
    uint8_t sender_id;
    uint8_t target_id;

    union {
        data_hello_t hello;
        data_welcome_t welcome;
        data_leave_t leave;
        data_disconnect_t disconnect;
        data_ping_t ping;
        data_pong_t pong;
        data_error_t error;
        data_set_ready_t set_ready;
        data_set_status_t set_status;
        data_winner_t winner;
        data_map_t map;
        data_move_attempt_t move_attempt;
        data_moved_t moved;
        data_bomb_attempt_t bomb_attempt;
        data_bomb_t bomb;
        data_explosion_t explosion;
        data_death_t death;
        data_bonus_available_t bonus_available;
        data_bonus_retrieved_t bonus_retrieved;
        data_block_destroyed_t block_destroyed;
    } data;
} Message;
