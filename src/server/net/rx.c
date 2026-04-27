#include "server/net/rx.h"
#include "server/net/args.h"
#include <pthread.h>

void* rx_thread(void* arg) {
    RxArgs* args = (RxArgs*)arg;
    while (1) {
        pthread_mutex_lock(&args->input->lock);

        pthread_mutex_unlock(&args->input->lock);
    }
    return NULL;
}
