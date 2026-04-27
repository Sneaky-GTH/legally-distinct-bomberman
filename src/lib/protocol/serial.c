#include "./messages.h"
#include <string.h>
#include <stdlib.h>

#define EPARSENODATA -1
#define EPARSENOMEM -2
#define EPARSEINVALID -3

static void read_uint8(const uint8_t *data, int *off, uint8_t *destination) {
    *destination = data[*off];
    (*off)++;
}

static void read_uint16(const uint8_t *data, int *off, uint16_t *destination) {
    *((uint8_t *)destination) = data[*off];
    (*off)++;
    *((uint8_t *)destination + 1) = data[*off];
    (*off)++;
}

static void read_cstrn(const uint8_t *data, int *off, char *destination, int max_len) {
    int start = *off;
    while (off - start < max_len && data[*off] != '\0') {
        destination[*off] = (char)data[*off];
        (*off)++;
    }

    if (*off < max_len) {
        destination[*off + 1] = '\0';
    }

    *off = start + max_len;
}

int parse_message(const uint8_t *data, int len, struct Message *msg) {
    if (len < 3)
        return EPARSENODATA; // needs at least the 3 byte header

    int off = 0;

    read_uint8(data, &off, &msg->type);
    read_uint8(data, &off, &msg->sender_id);
    read_uint8(data, &off, &msg->target_id);

    switch (msg->type) {
    case MSG_DISCONNECT:
    case MSG_PING:
    case MSG_PONG:
    case MSG_LEAVE:
    case MSG_SET_READY:
        return 3; // Degenerate messages

    case MSG_HELLO:
        if (len < 53)
            return EPARSENODATA; // HELLO is 53 bytes long
        read_cstrn(data, &off, &msg->data.hello.client_id, 20);
        read_cstrn(data, &off, &msg->data.hello.client_name, 30);
        return 53;

    case MSG_WELCOME:
        if (len < 25)
            return EPARSENODATA; // WELCOME is at least 25 bytes long
        read_cstrn(data, &off, &msg->data.welcome.server_name, 20);
        read_uint8(data, &off, &msg->data.welcome.status);
        read_uint8(data, &off, &msg->data.welcome.len);

        if (len < 25 + msg->data.welcome.len * 32)
            return EPARSENODATA;

        msg->data.welcome.clients = malloc(sizeof(clients_t) * msg->data.welcome.len);
        if (msg->data.welcome.clients == NULL) {
            return EPARSENOMEM;
        }

        for (int i = 0; i < msg->data.welcome.len; i++) {
            clients_t client = msg->data.welcome.clients[i];

            read_uint8(data, &off, &client.client_id);
            read_uint8(data, &off, &client.is_ready);
            read_cstrn(data, &off, &client.client_name, 30);
        }

        return off;

    case MSG_ERROR:
        if (len < 53)
            return EPARSENODATA; // ERROR is 53 bytes long
        read_cstrn(data, &off, &msg->data.error, 50);
        return 53;

    case MSG_SET_STATUS:
        if (len < 4)
            return EPARSENODATA; // SET_STATUS is 4 bytes long
        read_uint8(data, &off, &msg->data.set_status.status);
        return 4;

    case MSG_WINNER:
        if (len < 4)
            return EPARSENODATA; // WINNER is 4 bytes long
        read_uint8(data, &off, &msg->data.winner.winner_id);
        return 4;

    case MSG_MAP:
        if (len < 5)
            return EPARSENODATA; // MAP is at least 5 bytes long
        read_uint8(data, &off, msg->data.map.width);
        read_uint8(data, &off, msg->data.map.height);

        int map_len = msg->data.map.width * msg->data.map.height;

        if (len < map_len + 5)
            return EPARSENODATA;

        msg->data.map.map = malloc(sizeof(uint8_t) * map_len);
        if (msg->data.map.map == NULL) {
            return EPARSENOMEM;
        }

        for (int i = 0; i < map_len; i++) {
            read_uint8(data, &off, &msg->data.map.map);
        }

        return off;

    case MSG_MOVE_ATTEMPT:
        if (len < 4)
            return EPARSENODATA; // MOVE_ATTEMPT is 4 bytes long
        read_uint8(data, &off, &msg->data.move_attempt.direction);
        return 4;

    case MSG_MOVED:
        if (len < 6)
            return EPARSENODATA; // MOVED is 6 bytes long
        read_uint8(data, &off, &msg->data.moved.player_id);
        read_uint16(data, &off, &msg->data.moved.new_position);
        return 6;

    case MSG_BOMB_ATTEMPT:
        if (len < 5)
            return EPARSENODATA; // BOMB_ATTEMPT is 5 bytes long
        read_uint16(data, &off, &msg->data.bomb_attempt.position);
        return 5;

    case MSG_BOMB:
        if (len < 6)
            return EPARSENODATA; // BOMB is 6 bytes long
        read_uint8(data, &off, &msg->data.bomb.player_id);
        read_uint16(data, &off, &msg->data.bomb.position);
        return 6;

    case MSG_EXPLOSION_START:
    case MSG_EXPLOSION_END:
        if (len < 6)
            return EPARSENODATA; // EXPLOSION_START and EXPLOSION_END are 6 bytes long and identical
        read_uint8(data, &off, &msg->data.explosion.radius);
        read_uint16(data, &off, &msg->data.explosion.position);
        return 6;

    case MSG_DEATH:
        if (len < 4)
            return EPARSENODATA; // DEATH is 4 bytes long
        read_uint8(data, &off, &msg->data.death.player_id);
        return 4;

    case MSG_BONUS_AVAILABLE:
        if (len < 6)
            return EPARSENODATA; // BONUS_AVAILABLE is 6 bytes long
        read_uint8(data, &off, &msg->data.bonus_available.bonus_type);
        read_uint16(data, &off, &msg->data.bonus_available.position);
        return 6;

    case MSG_BONUS_RETRIEVED:
        if (len < 6)
            return EPARSENODATA; // BONUS_RETRIEVED is 6 bytes long
        read_uint8(data, &off, &msg->data.bonus_retrieved.player_id);
        read_uint16(data, &off, &msg->data.bonus_retrieved.position);
        return 6;

    case MSG_BLOCK_DESTROYED:
        if (len < 5)
            return EPARSENODATA; // BLOCK_DESTROYED is 5 bytes long
        read_uint16(data, &off, &msg->data.block_destroyed.position);
        return 5;

    default:
        return EPARSEINVALID;
    }
}
