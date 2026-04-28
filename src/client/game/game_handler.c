#include <stdlib.h>
#include <pthread.h>
#include "./handler.h"
#include "./state.h"
#include <string.h> // Why is memcpy in string.h and not in something like memory.h what the fuck??

static struct GameState GAME_STATE = {
    .width = 0,
    .height = 0,
    .field = NULL,
    .player_id = 0,
    .status = 0,
    .players = NULL,
};

const struct GameState* get_game_state() {
    return &GAME_STATE;
}

struct EventQueue {
    struct EventQueueNode *head;
    struct EventQueueNode *tail;
    
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
};

struct EventQueueNode {
    struct GameEvent event;
    struct EventQueueNode *next;
};

static struct EventQueue event_queue = {
    .head = NULL,
    .tail = NULL,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .not_empty = PTHREAD_COND_INITIALIZER
};

void enqueue_event(const struct GameEvent *event) {
    pthread_mutex_lock(&event_queue.lock);

    struct EventQueueNode *new_node = malloc(sizeof(struct EventQueueNode));
    memcpy(&new_node->event, event, sizeof(struct GameEvent));
    new_node->next = NULL;

    if (event_queue.tail) {
        event_queue.tail->next = new_node;
    } else {
        event_queue.head = new_node;
    }
    event_queue.tail = new_node;

    pthread_cond_signal(&event_queue.not_empty);
    pthread_mutex_unlock(&event_queue.lock);
}

// Blocks until an event is available and copies it to out_event
static void dequeue_event(struct GameEvent *out_event) {
    pthread_mutex_lock(&event_queue.lock);

    while (event_queue.head == NULL) {
        pthread_cond_wait(&event_queue.not_empty, &event_queue.lock);
    }

    struct EventQueueNode *node = event_queue.head;
    memcpy(out_event, &node->event, sizeof(struct GameEvent));
    event_queue.head = node->next;
    if (event_queue.head == NULL) {
        event_queue.tail = NULL;
    }

    free(node);
    pthread_mutex_unlock(&event_queue.lock);
}

void game_thread() {
    struct GameEvent event;

    for (;;) {
        dequeue_event(&event);
        // TODO
    }
}
