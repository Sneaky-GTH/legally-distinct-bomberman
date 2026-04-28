#include "./handler.h"
#include "./state.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Why is memcpy in string.h and not in something like memory.h what the fuck??
#include <sys/eventfd.h>
#include <unistd.h>

#ifdef DEBUG
#define LOG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif

static struct GameState GAME_STATE = {
    .width = 0,
    .height = 0,
    .field = NULL,
    .player_id = 0,
    .status = 0,
    .num_players = 0,
    .players = NULL,
    .explosion_tracker = { .tiles = NULL },
};

#define TILE_X(pos) ((pos) % GAME_STATE.width)
#define TILE_Y(pos) ((pos) / GAME_STATE.width)
#define POS(x, y) ((y) * GAME_STATE.width + (x))

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

struct Player* find_player_by_id(uint8_t player_id) {
    for (int i = 0; i < GAME_STATE.num_players; i++) {
        if (GAME_STATE.players[i].id == player_id) {
            return &GAME_STATE.players[i];
        }
    }
    return NULL;
}

void add_player(uint8_t player_id) {
    if (find_player_by_id(player_id) != NULL) {
        return; // Player already exists, do nothing
    }

    GAME_STATE.players = realloc(GAME_STATE.players, sizeof(struct Player) * (GAME_STATE.num_players + 1));
    GAME_STATE.players[GAME_STATE.num_players] = (struct Player) {
        .id = player_id,
        .p_count = 0,
        .p_size = 0,
        .p_speed = 0,
        .p_time = 0,
        .x = 0,
        .y = 0,
        .alive = true,
        .ready = false,
    };
    GAME_STATE.num_players++;
}

void remove_player(uint8_t player_id) {
    int index = -1;
    for (int i = 0; i < GAME_STATE.num_players; i++) {
        if (GAME_STATE.players[i].id == player_id) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        return; // Player not found, do nothing
    }

    // Move the last player in the array to the removed player's spot and shrink the array
    GAME_STATE.players[index] = GAME_STATE.players[GAME_STATE.num_players - 1];
    GAME_STATE.num_players--;
    GAME_STATE.players = realloc(GAME_STATE.players, sizeof(struct Player) * GAME_STATE.num_players);
}

static void ensure_explosion_tracker() {
    if (GAME_STATE.explosion_tracker.tiles) {
        return;
    }

    GAME_STATE.explosion_tracker.tiles = malloc(sizeof(struct ExplosionTileState) * GAME_STATE.width * GAME_STATE.height);
    for (int i = 0; i < GAME_STATE.width * GAME_STATE.height; i++) {
        GAME_STATE.explosion_tracker.tiles[i].cap = 0;
        GAME_STATE.explosion_tracker.tiles[i].len = 0;
        GAME_STATE.explosion_tracker.tiles[i].origins = NULL;
    }
}

static void add_explosion_origin(uint16_t position, uint16_t origin) {
    ensure_explosion_tracker();

    struct ExplosionTileState *tile_state = &GAME_STATE.explosion_tracker.tiles[position];
    if (tile_state->len == tile_state->cap) {
        tile_state->cap = tile_state->cap == 0 ? 4 : tile_state->cap * 2;
        tile_state->origins = realloc(tile_state->origins, sizeof(uint16_t) * tile_state->cap);
    }
    tile_state->origins[tile_state->len++] = origin;

    GAME_STATE.field[position] = EXPLOSION;
}

static void remove_explosion_origin(uint16_t origin) {
    for (int i = 0; i < GAME_STATE.width * GAME_STATE.height; i++) {
        struct ExplosionTileState *tile_state = &GAME_STATE.explosion_tracker.tiles[i];
        if (tile_state->len == 0) {
            continue;
        }
        for (int j = 0; j < tile_state->len; j++) {
            if (tile_state->origins[j] == origin) {
                // Remove this origin by replacing it with the last origin in the list
                tile_state->origins[j] = tile_state->origins[--tile_state->len];
                if (tile_state->len == 0) {
                    // No more origins, so this tile is no longer exploding. Set it back to whatever it should be based on the map data from the server
                    GAME_STATE.field[i] = GAME_STATE.field[i] == EXPLOSION ? EMPTY : GAME_STATE.field[i];
                }
                break;
            }
        }
    }
}

static void start_explosion(uint16_t position, uint8_t radius) {
    add_explosion_origin(position, position);

    uint8_t x = TILE_X(position);
    uint8_t y = TILE_Y(position);

    // Add explosion origins for all tiles in the explosion radius, stopping at hard walls, since explosions don't go through them
    // Note that the explosion radius is inclusive, so a radius 1 explosion affects the tile it's on and all adjacent tiles
    // Soft walls stop the explosion from going further, but they are still affected by the explosion, so we add the explosion origin for them as well before checking if we should stop
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            if (dx != 0 && dy != 0) {
                continue; // No diagonals
            }

            for (int r = 1; r <= radius; r++) {
                uint8_t target_x = x + dx * r;
                uint8_t target_y = y + dy * r;

                if (target_x >= GAME_STATE.width || target_y >= GAME_STATE.height) {
                    break;
                }

                uint16_t target_pos = POS(target_x, target_y);
                cell_types_t target_cell = GAME_STATE.field[target_pos];
                
                if (target_cell == HARD_WALL) {
                    break; // Stop at hard walls, but don't add explosion origin since they aren't affected by the explosion
                }

                add_explosion_origin(target_pos, position);

                if (target_cell == SOFT_WALL) {
                    break; // Stop at soft walls, and add explosion origin since they are affected by the explosion
                }
            }
        }
    }
}

static void end_explosion(uint16_t position, uint8_t radius) {
    (void) radius; // Radius is not needed for this
    remove_explosion_origin(position);
}

void game_thread() {
    struct GameEvent event;

    for (;;) {
        dequeue_event(&event);
        switch (event.type) {
            case EVENT_MOVE: {
                LOG("[GAME] Processing event: EVENT_MOVE");
                struct Player* player = find_player_by_id(event.move.player_id);
                if (!player) break;
                player->x = TILE_X(event.move.new_position);
                player->y = TILE_Y(event.move.new_position);
                break;
            }
            case EVENT_MOVE_ATTEMPT: {
                LOG("[GAME] Processing event: EVENT_MOVE_ATTEMPT");
                // Check if we can even move in that direction before sending the attempt to the server
                struct Player* player = find_player_by_id(GAME_STATE.player_id);
                if (!player) break;

                int8_t target_x = (int8_t) player->x;
                int8_t target_y = (int8_t) player->y;

                switch (event.move_attempt.dir) {
                    case DIR_UP:
                        target_y -= 1;
                        break;
                    case DIR_DOWN:
                        target_y += 1;
                        break;
                    case DIR_LEFT:
                        target_x -= 1;
                        break;
                    case DIR_RIGHT:
                        target_x += 1;
                        break;
                    default:
                        break;
                }

                // Check bounds
                if (target_x >= GAME_STATE.width || target_y >= GAME_STATE.height || target_x < 0 || target_y < 0) {
                    break;
                }

                // Check if the target cell is walkable before sending the attempt to the server
                cell_types_t target_cell = GAME_STATE.field[target_y * GAME_STATE.width + target_x];
                if (target_cell == HARD_WALL || target_cell == SOFT_WALL || target_cell == BOMB) {
                    break;
                }

                // Check if a player is already in the target cell before sending the attempt to the server
                bool player_in_cell = false;
                for (int i = 0; i < GAME_STATE.num_players; i++) {
                    if (GAME_STATE.players[i].x == target_x && GAME_STATE.players[i].y == target_y && GAME_STATE.players[i].alive) {
                        player_in_cell = true;
                        break;
                    }
                }
                if (player_in_cell) {
                    break;
                }

                // TODO: check for tickrate as well to prevent sending move attempts too quickly
                enqueue_net_event(&event);

                break;
            }

            case EVENT_PLACE_BOMB_ATTEMPT: {
                LOG("[GAME] Processing event: EVENT_PLACE_BOMB_ATTEMPT");
                struct Player* player = find_player_by_id(GAME_STATE.player_id);
                if (!player) break;
                if (GAME_STATE.field[POS(player->x, player->y)] == BOMB) {
                    break; // Can't place a bomb on top of another bomb
                }

                // TODO: check for bomb count when we know this attempt will be rejected by the server
                enqueue_net_event(&event);
                break;
            }
            case EVENT_PLACE_BOMB: {
                LOG("[GAME] Processing event: EVENT_PLACE_BOMB");
                GAME_STATE.field[event.place_bomb.position] = BOMB;
                break;
            }
            case EVENT_MAP: {
                LOG("[GAME] Processing event: EVENT_MAP");
                GAME_STATE.width = event.map.width;
                GAME_STATE.height = event.map.height;
                if (GAME_STATE.field) {
                    free(GAME_STATE.field);
                }
                GAME_STATE.field = event.map.field; // Assume ownership, since this is a copied pointer from the network thread that won't be used again
                // Clear player tiles and explosion tiles
                for (int i = 0; i < GAME_STATE.width * GAME_STATE.height; i++) {
                    switch (GAME_STATE.field[i]) {
                        case PLAYER_ONE:
                        case PLAYER_TWO:
                        case PLAYER_THREE:
                        case PLAYER_FOUR:
                        case PLAYER_FIVE:
                        case PLAYER_SIX:
                        case PLAYER_SEVEN:
                        case PLAYER_EIGHT:
                        case EXPLOSION:
                            GAME_STATE.field[i] = EMPTY;
                            break;
                        default:
                            break;
                    }
                }
                break;
            }

            case EVENT_NEW_PLAYER: {
                LOG("[GAME] Processing event: EVENT_NEW_PLAYER");
                if (find_player_by_id(event.new_player.player_id)) {
                    break; // Player already exists, do nothing
                }
                if (GAME_STATE.num_players == 0) {
                    // This is us!
                    GAME_STATE.player_id = event.new_player.player_id;
                }
                add_player(event.new_player.player_id);
                break;
            }

            case EVENT_PLAYER_REMOVED: {
                LOG("[GAME] Processing event: EVENT_PLAYER_REMOVED");
                remove_player(event.player_removed.player_id);
                break;
            }

            case EVENT_PLAYER_STATUS: {
                LOG("[GAME] Processing event: EVENT_PLAYER_STATUS");
                struct Player* player = find_player_by_id(event.player_status.player_id);
                if (!player) break;
                player->ready = event.player_status.ready;
                break;
            }

            case EVENT_SELF_READY: {
                LOG("[GAME] Processing event: EVENT_SELF_READY");
                struct Player* player = find_player_by_id(GAME_STATE.player_id);
                if (!player) break;
                player->ready = true;

                enqueue_net_event(&(struct GameEvent) { .type = EVENT_SELF_READY });
                break;
            }

            case EVENT_STATUS_UPDATE: {
                LOG("[GAME] Processing event: EVENT_STATUS_UPDATE");
                GAME_STATE.status = event.status_update.status;
                break;
            }

            case EVENT_WINNER: {
                LOG("[GAME] Processing event: EVENT_WINNER");
                // TODO: display winner in the UI using this event
                break;
            }

            case EVENT_EXPLOSION_START: {
                LOG("[GAME] Processing event: EVENT_EXPLOSION_START");
                start_explosion(event.explosion_start.position, event.explosion_start.radius);
                break;
            }

            case EVENT_EXPLOSION_END: {
                LOG("[GAME] Processing event: EVENT_EXPLOSION_END");
                end_explosion(event.explosion_end.position, event.explosion_end.radius);
                break;
            }

            case EVENT_DEATH: {
                LOG("[GAME] Processing event: EVENT_DEATH");
                struct Player* player = find_player_by_id(event.death.player_id);
                if (!player) break;
                player->alive = false;
                break;
            }

            case EVENT_BONUS_AVAILABLE: {
                LOG("[GAME] Processing event: EVENT_BONUS_AVAILABLE");
                // TODO: Handle bonus available event
                break;
            }

            case EVENT_BONUS_RETRIEVED: {
                LOG("[GAME] Processing event: EVENT_BONUS_RETRIEVED");
                // TODO: Handle bonus retrieved event
                break;
            }

            case EVENT_BLOCK_DESTROYED: {
                LOG("[GAME] Processing event: EVENT_BLOCK_DESTROYED");
                cell_types_t cell = GAME_STATE.field[event.block_destroyed.position];
                if (cell == SOFT_WALL) {
                    GAME_STATE.field[event.block_destroyed.position] = EMPTY;
                }
                break;
            }

            case EVENT_RESET: {
                LOG("[GAME] Processing event: EVENT_RESET");
                if (GAME_STATE.field) {
                    free(GAME_STATE.field);
                    GAME_STATE.field = NULL;
                }
                if (GAME_STATE.players) {
                    free(GAME_STATE.players);
                    GAME_STATE.players = NULL;
                }
                if (GAME_STATE.explosion_tracker.tiles) {
                    for (int i = 0; i < GAME_STATE.width * GAME_STATE.height; i++) {
                        if (GAME_STATE.explosion_tracker.tiles[i].origins) {
                            free(GAME_STATE.explosion_tracker.tiles[i].origins);
                        }
                    }
                    free(GAME_STATE.explosion_tracker.tiles);
                }
                GAME_STATE = (struct GameState) {
                    .width = 0,
                    .height = 0,
                    .field = NULL,
                    .player_id = GAME_STATE.player_id, // Preserve player ID across resets
                    .status = 0,
                    .num_players = 0,
                    .players = NULL,
                    .explosion_tracker = { .tiles = NULL },
                };
                break;
            }

            default:
                fprintf(stderr, "Unknown event type in game thread: %d\n", event.type);
                break;
        }
    }
}
