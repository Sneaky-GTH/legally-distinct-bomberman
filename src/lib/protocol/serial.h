#pragma once
#include "./messages.h"

int parse_message(const uint8_t *data, int len, struct Message *msg);
