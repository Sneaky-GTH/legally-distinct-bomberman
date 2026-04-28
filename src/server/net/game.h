#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <protocol/messages.h>
#include "server/net/args.h"
#include "server/logic/playingfield.h"
#include "server/logic/player.h"
#include "server/logic/bomb.h"
#include <time.h>


typedef struct Client {
    Player p;
    int fd;
    int ready;
} Client;

typedef struct GameState {
    Client clients[MAX_CLIENTS];
    PlayingField wallmap;
    PlayingField playermap;
    Explosion* explosions;
    Bomb* bombs;
    Antibomb* antibombs;
    int status;
} GameState;


int add_new_client(Client clients[MAX_CLIENTS], int fd);
int remove_client(Client clients[MAX_CLIENTS], int id);

void send_to_client(int c, struct Message msg, MessageQueue* output);
void broadcast_to_clients(Client clients[MAX_CLIENTS], Message msg, MessageQueue* output);
void process_action(ClientMessage* msg, MessageQueue* queue);

void *game_thread(void* arg);


#endif
