#include <stdint.h>
#include <protocol/map.h>
#include <bool.h>

struct Player {
    uint8_t id;
    uint8_t p_count; // Bomb count
    uint8_t p_size;  // Bomb explosion size
    uint8_t p_speed; // Movement speed (ticks per movement event)
    uint8_t p_time;  // Bomb timer (ticks until explosion)
    uint8_t x;
    uint8_t y;
    bool alive; // true if alive, false if dead
    bool ready; // true if player has sent ready signal, false otherwise
};

struct ExplosionTileState {
    uint8_t cap;
    uint8_t len;
    uint16_t* origins;
};

struct ExplosionTracker {
    // This is a bit complex, so an explanation is needed.
    // Basically, for every tile in GAME_STATE.field, mark explosion tiles from which they originated
    // Note that a tile may have multiple origins, and the tile should stop being "exploded" when all
    // its origins are no longer active
    struct ExplosionTileState* tiles;
};

struct GameState {
    // Max is 255 (limit of uint8)
    // If any of these are 0, the map is not yet received from the server and the game board should not be rendered
    uint8_t width;
    uint8_t height;
    // 2D array of cell types, flattened into 1D (row-major order)
    // Implicit "H" cells outside the bounds of the map
    cell_types_t *field;

    uint8_t player_id;
    // 0 = lobby, 1 = game in progress, 2 = game ended
    uint8_t status;

    uint8_t num_players;
    struct Player* players; // Dynamically allocated array of player states
    struct ExplosionTracker explosion_tracker;
};

const struct GameState* get_game_state();
