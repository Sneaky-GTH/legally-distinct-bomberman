#pragma once

#include <stdint.h>

#define msg_common_fields                                                                          \
    msg_type_t msg_type;                                                                           \
    uint8_t sender_id;                                                                             \
    uint8_t target_id

#define def_degenerate_msg(name)                                                                   \
    typedef struct {                                                                               \
        msg_common_fields;                                                                         \
    } msg_##name##_t;

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
    MSG_BLOCK_DESTROYED = 47,
    MSG_SYNC_BOARD = 100,
    MSG_SYNC_REQUEST = 101
} msg_type_t;

typedef struct {
    msg_common_fields;
    char client_id[20];
    char client_name[30];
} msg_hello_t;

typedef struct {
    uint8_t client_id;
    bool is_ready;
    char client_name[30];
} clients_t;

typedef struct {
    msg_common_fields;
    char server_name[20];
    game_status_t status;
    uint8_t len;
    clients_t clients[];
} msg_welcome_t;

def_degenerate_msg(leave);
def_degenerate_msg(disconnect);
def_degenerate_msg(ping);
def_degenerate_msg(pong);

typedef struct {
    msg_common_fields;
    uint8_t error_len; // TODO: non-standard
    char error_message[];
} msg_ready_t;

def_degenerate_msg(set_ready);

typedef struct {
    msg_common_fields;
    uint8_t status;
} msg_set_status_t;

typedef struct {
    msg_common_fields;
    uint8_t winner_id;
} msg_winner_t;

typedef struct {
    msg_common_fields;
    uint8_t height;
    uint8_t width;
    uint8_t map[];
} msg_map_t;

typedef struct {
    msg_common_fields;
    direction_t direction;
} msg_move_attempt_t;

typedef struct {
    msg_common_fields;
    uint8_t player_id;
    uint16_t new_position;
} msg_moved_t;

typedef struct {
    msg_common_fields;
    uint16_t position;
} msg_bomb_attempt_t;

typedef struct {
    msg_common_fields;
    uint8_t player_id;
    uint16_t position;
} msg_bomb_t;

typedef struct {
    msg_common_fields;
    uint8_t radius;
    uint16_t position;
} msg_explosion_start_t;

typedef struct {
    msg_common_fields;
    uint16_t position;
} msg_explosion_end_t;

typedef struct {
    msg_common_fields;
    uint8_t player_id;
} msg_death_t;

typedef struct {
    msg_common_fields;
    bonus_type_t bonus_type;
    uint16_t position;
} msg_bonus_available_t;

typedef struct {
    msg_common_fields;
    uint8_t player_id;
    uint16_t position;
} msg_bonus_retrieved_t;

typedef struct {
    msg_common_fields;
    uint16_t position;
} msg_block_destroyed_t;
