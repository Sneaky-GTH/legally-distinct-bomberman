// args.h ───────────────────────────────────────────────
#ifndef ARGS_H
#define ARGS_H
#include "lib/protocol/messages.h"
#include <pthread.h>

#define MAX_QUEUE 256

typedef struct {
    Message messages[MAX_QUEUE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t  not_empty;
} MessageQueue;

typedef struct {
    MessageQueue *input;
    int epfd;
} RxArgs;

typedef struct {
    MessageQueue *input;
    MessageQueue *output;
} GameArgs;

typedef struct {
    MessageQueue *output;
} TxArgs;

#endif
