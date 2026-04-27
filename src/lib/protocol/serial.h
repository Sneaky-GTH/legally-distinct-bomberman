#pragma once
#include "./messages.h"

#define EREADNODATA -1
#define EREADNOMEM -2
#define EREADINVALID -3

#define EWRITENOSPACE -1

int read_message(const uint8_t *data, int len, struct Message *msg);
int write_message(uint8_t *buf, int buf_size, const struct Message *msg);
