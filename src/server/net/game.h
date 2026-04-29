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
    int is_ready;
    int is_alive;
    char name[30];
} Client;

typedef struct GameState {
    Client clients[MAX_CLIENTS];
    int client_count;
    PlayingField wallmap;
    PlayingField playermap;
    Explosion* explosions;
    Bomb* bombs;
    Antibomb* antibombs;
    int status;
} GameState;


void send_to_client(int c, struct Message msg, MessageQueue* output);
void broadcast_to_clients(Client clients[MAX_CLIENTS], Message msg, MessageQueue* output);
void process_action(ClientMessage* msg, MessageQueue* queue);
void spread_out_players(GameState* game, MessageQueue* output);
void check_player_death(GameState* game, ServerMessage* servermessages);

void *game_thread(void* arg);


#endif
