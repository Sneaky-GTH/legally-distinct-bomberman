#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <protocol/messages.h>
#include "server/net/args.h"
#include "server/logic/playingfield.h"
#include "server/logic/player.h"

typedef struct {
    Player p;
    int fd;
    int id;
} Client;

typedef struct {
    Client clients[MAX_CLIENTS];
    PlayingField field;
} GameState;

void send_to_client(int c, struct Message msg, MessageQueue* output);
void process_action(ClientMessage* msg, MessageQueue* queue);

void *game_thread(void* arg);


#endif
