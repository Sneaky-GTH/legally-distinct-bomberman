#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "server/logic/messagehandling.h"
#include "server/net/game.h"

Message srv_process_hello(GameState* game, Message* msg) {


    Message return_msg = {
        .type = MSG_WELCOME,
        .sender_id = 255,
        .target_id = 0,
        .data.welcome = {
            .server_name = "bomboclat-express",
            .status = GAME_LOBBY,
            .len = 0,
            .clients = NULL
        },
    };


    return return_msg;


}

