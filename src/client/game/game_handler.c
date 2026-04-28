#include <stdlib.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <unistd.h>
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

static struct EventQueue net_event_queue = {
    .head = NULL,
    .tail = NULL,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .not_empty = PTHREAD_COND_INITIALIZER, // Actually unused
};

static int net_event_fd = -1;

int get_net_event_fd(void) {
    if (net_event_fd == -1) {
        net_event_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    }
    return net_event_fd;
}

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

static void enqueue_net_event(const struct GameEvent *event) {
    pthread_mutex_lock(&net_event_queue.lock);

    struct EventQueueNode *new_node = malloc(sizeof(struct EventQueueNode));
    memcpy(&new_node->event, event, sizeof(struct GameEvent));
    new_node->next = NULL;

    if (net_event_queue.tail) {
        net_event_queue.tail->next = new_node;
    } else {
        net_event_queue.head = new_node;
    }
    net_event_queue.tail = new_node;

    uint64_t one = 1;
    write(get_net_event_fd(), &one, sizeof(one));

    pthread_cond_signal(&net_event_queue.not_empty);
    pthread_mutex_unlock(&net_event_queue.lock);
}

bool dequeue_net_event(struct GameEvent *out_event) {
    pthread_mutex_lock(&net_event_queue.lock);

    if (net_event_queue.head == NULL) {
        pthread_mutex_unlock(&net_event_queue.lock);
        return false;
    }

    struct EventQueueNode *node = net_event_queue.head;
    memcpy(out_event, &node->event, sizeof(struct GameEvent));
    net_event_queue.head = node->next;
    if (net_event_queue.head == NULL) {
        net_event_queue.tail = NULL;
    }

    free(node);
    pthread_mutex_unlock(&net_event_queue.lock);
    return true;
}

void game_thread() {
    struct GameEvent event;

    for (;;) {
        dequeue_event(&event);
        // TODO
    }
}
