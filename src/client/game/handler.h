#pragma once
#include <bool.h>
#include <net.h>

struct GameEvent {
    // *_ATTEMPT messages are received from render thread, may be rejected by server
    // Other messages are received from network thread, guaranteed to be valid and should be processed by game thread
    enum {
        EVENT_MOVE,
        EVENT_MOVE_ATTEMPT,
        EVENT_PLACE_BOMB_ATTEMPT, // No data
        EVENT_PLACE_BOMB,
        EVENT_MAP,
        EVENT_NEW_PLAYER,
        EVENT_PLAYER_REMOVED,
        EVENT_PLAYER_STATUS,
        EVENT_SELF_READY, // No data, signals that we should toggle our own ready status and send update to server
        EVENT_STATUS_UPDATE,
        EVENT_RESET, // No data, signals game reset
        EVENT_WINNER,
        EVENT_EXPLOSION_START,
        EVENT_EXPLOSION_END,
        EVENT_DEATH,
        EVENT_BONUS_AVAILABLE,
        EVENT_BONUS_RETRIEVED,
        EVENT_BLOCK_DESTROYED,
    } type;

    union {
        struct {
            uint8_t player_id;
            uint16_t new_position;
        } move;

        struct {
            direction_t dir;
        } move_attempt;

        struct {
            uint16_t position;
        } place_bomb;

        struct {
            uint16_t position;
            uint8_t radius;
        } explosion_start;

        struct {
            uint8_t width;
            uint8_t height;
            cell_types_t *field; // Dynamically allocated, must be freed
        } map;

        struct {
            uint8_t player_id;
            char name[30];
        } new_player;

        struct {
            uint8_t player_id;
        } player_removed;

        struct {
            uint8_t player_id;
            bool ready; // true if player is ready, false otherwise
        } player_status;

        struct {
            uint8_t status; // 0 = lobby, 1 = game in progress, 2 = game ended
        } status_update;

        struct {
            uint8_t winner_id;
        } winner;

        struct {
            uint8_t radius;
            uint16_t position;
        } explosion_end;

        struct {
            uint8_t player_id;
        } death;

        struct {
            bonus_type_t bonus_type;
            uint16_t position;
        } bonus_available;

        struct {
            uint8_t player_id;
            uint16_t position;
        } bonus_retrieved;

        struct {
            uint16_t position;
        } block_destroyed;
    };
};

void enqueue_event(const struct GameEvent *event);
// If an event is available, writes it to out_event and returns true. Otherwise, returns false.
bool dequeue_net_event(struct GameEvent *event);
int get_net_event_fd(void);

