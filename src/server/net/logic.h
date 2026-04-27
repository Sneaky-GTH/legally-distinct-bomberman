#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <protocol/messages.h>

void process_action(struct Message* msg);

void *game_thread(void* arg);


#endif
