#ifndef MESSAGE_HANDLING_H
#define MESSAGE_HANDLING_H

#include <stdint.h>
#include <protocol/messages.h>
#include "server/logic/playingfield.h"
#include "server/net/game.h"

Message srv_process_hello(GameState* game, Message* msg);

#endif
