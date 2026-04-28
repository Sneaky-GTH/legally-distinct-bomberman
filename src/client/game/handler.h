#pragma once
#include <net.h>

struct GameEvent {
    // *_ATTEMPT messages are recieved from render thread, may be rejected by server
    // Other messages are recieved from network thread, guaranteed to be valid and should be processed by game thread
    enum {
        EVENT_MOVE,
        EVENT_MOVE_ATTEMPT,
        EVENT_PLACE_BOMB_ATTEMPT, // No data
        EVENT_PLACE_BOMB,
        EVENT_MAP,
        EVENT_NEW_PLAYER,
        EVENT_PLAYER_REMOVED,
        EVENT_PLAYER_STATUS,
        EVENT_STATUS_UPDATE,
        EVENT_RESET, // No data, signals game reset
    } type;

    union {
        struct {
            uint8_t player_id;
            uint8_t x;
            uint8_t y;
        } move;

        struct {
            direction_t dir;
        } move_attempt;

        struct {
            uint8_t x;
            uint8_t y;
        } place_bomb;

        struct {
            uint8_t width;
            uint8_t height;
            cell_types_t *field; // Dynamically allocated, must be freed
        } map;

        struct {
            uint8_t player_id;
            char name[256];
        } new_player;

        struct {
            uint8_t player_id;
        } player_removed;

        struct {
            uint8_t player_id;
            uint8_t status; // 0 = lobby, 1 = game in progress, 2 = game ended
        } status_update;
    };
};

void enqueue_event(const struct GameEvent *events);
