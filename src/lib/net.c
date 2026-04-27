#include "./net.h"
#include "./protocol/messages.h"
#include "./protocol/serial.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int send_all(int sock, const uint8_t *buf, int len) {
    int total = 0;
    while (total < len) {
        int n = send(sock, buf + total, len - total, 0);
        if (n == -1) return -1;
        if (n == 0) return -1;
        total += n;
    }
    return 0;
}

int send_message(int sock, const Message *msg) {
    uint8_t buf[4096];
    int len = write_message(buf, sizeof(buf), msg);
    if (len < 0) return len;
    return send_all(sock, buf, len);
}

/*
 * Receives a message from the socket and parses it into `msg`.
 * Handles multiple messages in one packet and incomplete fragments.
 * Example usage:
 * 
 *     uint8_t buf[4096]; // Pre-allocated buffer
 *     int valid_len = 0; // Tracks unparsed data length across calls
 *     Message msg;
 *     
 *     // In a loop...
 *     int bytes_consumed = recv_message(sock, buf, &valid_len, sizeof(buf), &msg);
 *     
 *     if (bytes_consumed > 0) {
 *         // Successfully received and parsed message! (buffer shifts automatically)
 *         printf("Received message type: %d\n", msg.type);
 *         free_message(&msg);
 *     } else if (bytes_consumed == ERECVCLOSED) {
 *         // Handle disconnect
 *     }
 */
int recv_message(int sock, uint8_t *buf, int *buf_len, int buf_cap, Message *msg) {
    // First, try to parse from already buffered data
    int res = read_message(buf, *buf_len, msg);
    if (res > 0) {
        int remaining = *buf_len - res;
        memmove(buf, buf + res, remaining);
        *buf_len = remaining;
        return res;
    }
    if (res < 0 && res != EREADNODATA) return res;

    // Receive loop: keep fetching until we parse a message or hit an error/cap
    while (*buf_len < buf_cap) {
        int n = recv(sock, buf + *buf_len, buf_cap - *buf_len, 0);
        if (n == -1) return ERECVTIMEOUT; // TODO: handle partial timeouts better/EAGAIN
        if (n == 0) return ERECVCLOSED;
        *buf_len += n;

        int parse_res = read_message(buf, *buf_len, msg);
        if (parse_res > 0) {
            int remaining = *buf_len - parse_res;
            memmove(buf, buf + parse_res, remaining);
            *buf_len = remaining;
            return parse_res;
        }
        if (parse_res < 0 && parse_res != EREADNODATA) return parse_res;
    }
    
    return ERECVFULL;
}

void free_message(Message *msg) {
    if (!msg) return;
    if (msg->type == MSG_WELCOME) {
        if (msg->data.welcome.clients) {
            free(msg->data.welcome.clients);
            msg->data.welcome.clients = NULL;
        }
    } else if (msg->type == MSG_ERROR) {
        if (msg->data.error.error_message) {
            free(msg->data.error.error_message);
            msg->data.error.error_message = NULL;
        }
    } else if (msg->type == MSG_MAP) {
        if (msg->data.map.map) {
            free(msg->data.map.map);
            msg->data.map.map = NULL;
        }
    }
}
