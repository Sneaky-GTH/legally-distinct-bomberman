#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <protocol/messages.h>
#include "server/net/args.h"

void process_action(ClientMessage* msg, MessageQueue* queue);

void *game_thread(void* arg);


#endif
