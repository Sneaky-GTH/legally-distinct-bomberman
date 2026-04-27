#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "lib.h"
#include <protocol/messages.h>
#include "server/game/playingfield.h"
#include "server/game/player.h"
#include "server/net/args.h"

#define MSG_SIZE 4096


int main(void) {

    MessageQueue input_queue  = {
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .not_empty = PTHREAD_COND_INITIALIZER
    };

    MessageQueue output_queue = {
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .not_empty = PTHREAD_COND_INITIALIZER
    };

    //int epfd = setup_epoll();

    // each thread gets only what it needs
    //RxArgs rx_args = { .input  = &input_queue, .epfd = epfd };
    GameArgs game_args = { .input = &input_queue, .output = &output_queue };
    //TxArgs tx_args = { .output = &output_queue};

    pthread_t rx_tid, game_tid, tx_tid;
    //pthread_create(&rx_tid,   NULL, rx_thread,   &rx_args);
    //pthread_create(&game_tid, NULL, game_thread, &game_args);
    //pthread_create(&tx_tid,   NULL, tx_thread,   &tx_args);

    //pthread_join(rx_tid,   NULL);
    //pthread_join(game_tid, NULL);
    //pthread_join(tx_tid,   NULL);

    return 0;

}
