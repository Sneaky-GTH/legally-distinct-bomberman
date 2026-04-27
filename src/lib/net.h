#pragma once
#include <stdint.h>

#define ERECVFULL -1
#define ERECVCLOSED -2
#define ERECVTIMEOUT -3

int send_all(int sock, const uint8_t *buf, int len);

// Returns 0 on success
// TODO: errors
int send_message(int sock, const struct Message *msg);

// Returns number of bytes consumed (length of message), throwing ERECV* on error
// buf_len specifies valid bytes currently in buf and is updated with leftovers on return
int recv_message(int sock, uint8_t *buf, int *buf_len, int buf_cap, struct Message *msg);

// Frees any dynamically allocated memory in the message (e.g. data_welcome_t.clients)
// Does not free the message itself, since it may be stack-allocated.
void free_message(struct Message *msg);

