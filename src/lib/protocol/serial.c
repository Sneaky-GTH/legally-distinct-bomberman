#include "./messages.h"
#include "./serial.h"
#include <string.h>
#include <stdlib.h>

static void read_uint8(const uint8_t *data, int *off, uint8_t *destination) {
    *destination = data[(*off)++];
}

static void write_uint8(uint8_t *buf, int *off, const uint8_t value) {
    buf[(*off)++] = value;
}

static void read_uint16(const uint8_t *data, int *off, uint16_t *destination) {
    *destination = ((uint16_t)data[*off] << 8) | data[*off + 1];
    *off += 2;
}

static void write_uint16(uint8_t *buf, int *off, const uint16_t value) {
    buf[(*off)++] = (value >> 8) & 0xFF;
    buf[(*off)++] = value & 0xFF;
}

static void read_cstrn(const uint8_t *data, int *off, char *destination, int max_len) {
    int start = *off;
    while (*off - start < max_len && data[*off] != '\0') {
        destination[*off] = (char)data[*off];
        (*off)++;
    }

    if (*off < max_len) {
        destination[*off + 1] = '\0';
    }

    *off = start + max_len;
}

static void write_cstrn(uint8_t *buf, int *off, const char *value, int max_len) {
    int written = 0;
    while (written < max_len && value[written] != '\0') {
        buf[*off + written] = (uint8_t) value[written];
        written++;
    }

    while (written < max_len) {
        buf[*off + written] = (char)'\0';
        written++;
    }

    *off += written;
}

int read_message(const uint8_t *data, int len, struct Message *msg) {
    if (len < 3)
        return EREADNODATA; // needs at least the 3 byte header

    int off = 0;

    read_uint8(data, &off, (uint8_t*) &msg->type);
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
            return EREADNODATA; // HELLO is 53 bytes long
        read_cstrn(data, &off, msg->data.hello.client_id, 20);
        read_cstrn(data, &off, msg->data.hello.client_name, 30);
        return 53;

    case MSG_WELCOME:
        if (len < 25)
            return EREADNODATA; // WELCOME is at least 25 bytes long
        read_cstrn(data, &off, msg->data.welcome.server_name, 20);
        read_uint8(data, &off, (uint8_t*) &msg->data.welcome.status);
        read_uint8(data, &off, &msg->data.welcome.len);

        if (len < 25 + msg->data.welcome.len * 32)
            return EREADNODATA;

        msg->data.welcome.clients = malloc(sizeof(clients_t) * msg->data.welcome.len);
        if (msg->data.welcome.clients == NULL) {
            return EREADNOMEM;
        }

        for (int i = 0; i < msg->data.welcome.len; i++) {
            clients_t client = msg->data.welcome.clients[i];

            read_uint8(data, &off, &client.client_id);
            read_uint8(data, &off, &client.is_ready);
            read_cstrn(data, &off, client.client_name, 30);
        }

        return off;

    case MSG_ERROR:
        if (len < 53)
            return EREADNODATA; // ERROR is 53 bytes long
        read_cstrn(data, &off, msg->data.error.error_message, 50);
        return 53;

    case MSG_SET_STATUS:
        if (len < 4)
            return EREADNODATA; // SET_STATUS is 4 bytes long
        read_uint8(data, &off, &msg->data.set_status.status);
        return 4;

    case MSG_WINNER:
        if (len < 4)
            return EREADNODATA; // WINNER is 4 bytes long
        read_uint8(data, &off, &msg->data.winner.winner_id);
        return 4;

    case MSG_MAP:
        if (len < 5)
            return EREADNODATA; // MAP is at least 5 bytes long
        read_uint8(data, &off, &msg->data.map.width);
        read_uint8(data, &off, &msg->data.map.height);

        int map_len = msg->data.map.width * msg->data.map.height;

        if (len < map_len + 5)
            return EREADNODATA;

        msg->data.map.map = malloc(sizeof(uint8_t) * map_len);
        if (msg->data.map.map == NULL) {
            return EREADNOMEM;
        }

        for (int i = 0; i < map_len; i++) {
            read_uint8(data, &off, (uint8_t*) &(msg->data.map.map[i]));
        }

        return off;

    case MSG_MOVE_ATTEMPT:
        if (len < 4)
            return EREADNODATA; // MOVE_ATTEMPT is 4 bytes long
        read_uint8(data, &off, (uint8_t*) &msg->data.move_attempt.direction);
        return 4;

    case MSG_MOVED:
        if (len < 6)
            return EREADNODATA; // MOVED is 6 bytes long
        read_uint8(data, &off, &msg->data.moved.player_id);
        read_uint16(data, &off, &msg->data.moved.new_position);
        return 6;

    case MSG_BOMB_ATTEMPT:
        if (len < 5)
            return EREADNODATA; // BOMB_ATTEMPT is 5 bytes long
        read_uint16(data, &off, &msg->data.bomb_attempt.position);
        return 5;

    case MSG_BOMB:
        if (len < 6)
            return EREADNODATA; // BOMB is 6 bytes long
        read_uint8(data, &off, &msg->data.bomb.player_id);
        read_uint16(data, &off, &msg->data.bomb.position);
        return 6;

    case MSG_EXPLOSION_START:
    case MSG_EXPLOSION_END:
        if (len < 6)
            return EREADNODATA; // EXPLOSION_START and EXPLOSION_END are 6 bytes long and identical
        read_uint8(data, &off, &msg->data.explosion.radius);
        read_uint16(data, &off, &msg->data.explosion.position);
        return 6;

    case MSG_DEATH:
        if (len < 4)
            return EREADNODATA; // DEATH is 4 bytes long
        read_uint8(data, &off, &msg->data.death.player_id);
        return 4;

    case MSG_BONUS_AVAILABLE:
        if (len < 6)
            return EREADNODATA; // BONUS_AVAILABLE is 6 bytes long
        read_uint8(data, &off, (uint8_t*) &msg->data.bonus_available.bonus_type);
        read_uint16(data, &off, &msg->data.bonus_available.position);
        return 6;

    case MSG_BONUS_RETRIEVED:
        if (len < 6)
            return EREADNODATA; // BONUS_RETRIEVED is 6 bytes long
        read_uint8(data, &off, &msg->data.bonus_retrieved.player_id);
        read_uint16(data, &off, &msg->data.bonus_retrieved.position);
        return 6;

    case MSG_BLOCK_DESTROYED:
        if (len < 5)
            return EREADNODATA; // BLOCK_DESTROYED is 5 bytes long
        read_uint16(data, &off, &msg->data.block_destroyed.position);
        return 5;

    default:
        return EREADINVALID;
    }
}

int write_message(uint8_t *buf, int buf_size, const struct Message *msg) {
    if (buf_size < 3)
        return EWRITENOSPACE;

    int off = 0;

    write_uint8(buf, &off, msg->type);
    write_uint8(buf, &off, msg->sender_id);
    write_uint8(buf, &off, msg->target_id);

    switch (msg->type) {
    case MSG_DISCONNECT:
    case MSG_PING:
    case MSG_PONG:
    case MSG_LEAVE:
    case MSG_SET_READY:
        return 3; // Degenerate messages

    case MSG_HELLO:
        if (buf_size < 53)
            return EWRITENOSPACE; // HELLO is 53 bytes long
        write_cstrn(buf, &off, msg->data.hello.client_id, 20);
        write_cstrn(buf, &off, msg->data.hello.client_name, 30);
        return 53;

    case MSG_WELCOME:
        if (buf_size < 25)
            return EWRITENOSPACE; // WELCOME is at least 25 bytes long
        write_cstrn(buf, &off, msg->data.welcome.server_name, 20);
        write_uint8(buf, &off, msg->data.welcome.status);
        write_uint8(buf, &off, msg->data.welcome.len);

        if (buf_size < 25 + msg->data.welcome.len * 32)
            return EWRITENOSPACE;

        for (int i = 0; i < msg->data.welcome.len; i++) {
            write_uint8(buf, &off, msg->data.welcome.clients[i].client_id);
            write_uint8(buf, &off, msg->data.welcome.clients[i].is_ready);
            write_cstrn(buf, &off, msg->data.welcome.clients[i].client_name, 30);
        }

        return off;

    case MSG_ERROR:
        if (buf_size < 53)
            return EWRITENOSPACE; // ERROR is 53 bytes long
        write_cstrn(buf, &off, msg->data.error.error_message, 50);
        return 53;

    case MSG_SET_STATUS:
        if (buf_size < 4)
            return EWRITENOSPACE; // SET_STATUS is 4 bytes long
        write_uint8(buf, &off, msg->data.set_status.status);
        return 4;

    case MSG_WINNER:
        if (buf_size < 4)
            return EWRITENOSPACE; // WINNER is 4 bytes long
        write_uint8(buf, &off, msg->data.winner.winner_id);
        return 4;

    case MSG_MAP:
        if (buf_size < 5)
            return EWRITENOSPACE; // MAP is at least 5 bytes long
        write_uint8(buf, &off, msg->data.map.width);
        write_uint8(buf, &off, msg->data.map.height);

        int map_len = msg->data.map.width * msg->data.map.height;

        if (buf_size < map_len + 5)
            return EWRITENOSPACE;

        for (int i = 0; i < map_len; i++) {
            write_uint8(buf, &off, msg->data.map.map[i]);
        }

        return off;

    case MSG_MOVE_ATTEMPT:
        if (buf_size < 4)
            return EWRITENOSPACE; // MOVE_ATTEMPT is 4 bytes long
        write_uint8(buf, &off, msg->data.move_attempt.direction);
        return 4;

    case MSG_MOVED:
        if (buf_size < 6)
            return EWRITENOSPACE; // MOVED is 6 bytes long
        write_uint8(buf, &off, msg->data.moved.player_id);
        write_uint16(buf, &off, msg->data.moved.new_position);
        return 6;

    case MSG_BOMB_ATTEMPT:
        if (buf_size < 5)
            return EWRITENOSPACE; // BOMB_ATTEMPT is 5 bytes long
        write_uint16(buf, &off, msg->data.bomb_attempt.position);
        return 5;

    case MSG_BOMB:
        if (buf_size < 6)
            return EWRITENOSPACE; // BOMB is 6 bytes long
        write_uint8(buf, &off, msg->data.bomb.player_id);
        write_uint16(buf, &off, msg->data.bomb.position);
        return 6;

    case MSG_EXPLOSION_START:
    case MSG_EXPLOSION_END:
        if (buf_size < 6)
            return EWRITENOSPACE; // EXPLOSION_START and EXPLOSION_END are 6 bytes long and identical
        write_uint8(buf, &off, msg->data.explosion.radius);
        write_uint16(buf, &off, msg->data.explosion.position);
        return 6;

    case MSG_DEATH:
        if (buf_size < 4)
            return EWRITENOSPACE; // DEATH is 4 bytes long
        write_uint8(buf, &off, msg->data.death.player_id);
        return 4;

    case MSG_BONUS_AVAILABLE:
        if (buf_size < 6)
            return EWRITENOSPACE; // BONUS_AVAILABLE is 6 bytes long
        write_uint8(buf, &off, msg->data.bonus_available.bonus_type);
        write_uint16(buf, &off, msg->data.bonus_available.position);
        return 6;

    case MSG_BONUS_RETRIEVED:
        if (buf_size < 6)
            return EWRITENOSPACE; // BONUS_RETRIEVED is 6 bytes long
        write_uint8(buf, &off, msg->data.bonus_retrieved.player_id);
        write_uint16(buf, &off, msg->data.bonus_retrieved.position);
        return 6;

    case MSG_BLOCK_DESTROYED:
        if (buf_size < 5)
            return EWRITENOSPACE; // BLOCK_DESTROYED is 5 bytes long
        write_uint16(buf, &off, msg->data.block_destroyed.position);
        return 5;

    default:
        return EREADINVALID;
    }
}

