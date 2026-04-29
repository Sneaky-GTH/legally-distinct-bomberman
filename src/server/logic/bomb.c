#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "bomb.h"
#include "playingfield.h"
#include "server/logic/playingfield.h"
#include "server/logic/bomb.h"
#include "lib/protocol/messages.h"
#include "server/net/game.h"
#include "server/net/args.h"


void add_antibomb(GameState* game, Antibomb* antibomb) {
    Antibomb* currentbomb = game->antibombs;

    if (currentbomb == NULL) {
        game->antibombs = antibomb;
        return;
    }


    while (currentbomb->nextantibomb != NULL) {
        currentbomb = currentbomb->nextantibomb;
    }


    currentbomb->nextantibomb = antibomb;

}



int create_antibomb(GameState* game, uint8_t target_x, uint8_t target_y, int r) {

    Antibomb* antibomb = malloc(sizeof(Antibomb));

    antibomb->x = target_x;
    antibomb->y = target_y;
    antibomb->lifetime = DEFAULT_ANTIBOMB_TIMER;
    antibomb->radius = r;

    add_antibomb(game, antibomb);
}



int explode_bomb(GameState* game, Bomb* bomb, ServerMessage* servermessages) {

    printf("GAME INFO: Trying to explode bomb....");

    PlayingField* field = &game->wallmap;
    uint8_t x = bomb->x;
    uint8_t y = bomb->y;
    uint8_t r = bomb->radius + 1;
    uint8_t lifetime = bomb->lifetime;

    create_antibomb(game, x, y, r);

    if (SAFE_GET_CELL(field, x, y) != 'B') {
        return -1;
    }

    SAFE_SET_CELL(field, x, y, 'X');
    Explosion* explo = create_explosion(x, y, bomb);
    add_explosion(game, explo);


    for (int i = 1; i < r; i++) {
        if (SAFE_GET_CELL(field, x+i, y) == 'f') break;
        if (SAFE_GET_CELL(field, x+i, y) == 'H') break;
        if (SAFE_GET_CELL(field, x+i, y) == 'S') {
            SAFE_SET_CELL(field, x+i, y, '.');
            break;
        }
        SAFE_SET_CELL(field, x+i, y, 'X');
        Explosion* explo = create_explosion(x+i, y, bomb);
        add_explosion(game, explo);
    }

    for (int i = 1; i < r; i++) {
        if (SAFE_GET_CELL(field, x-i, y) == 'f') break;
        if (SAFE_GET_CELL(field, x-i, y) == 'H') break;
        if (SAFE_GET_CELL(field, x-i, y) == 'S') {
            SAFE_SET_CELL(field, x-i, y, '.');
            break;
        }
        SAFE_SET_CELL(field, x-i, y, 'X');
        Explosion* explo = create_explosion(x-i, y, bomb);
        add_explosion(game, explo);
    }

    for (int i = 1; i < r; i++) {
        if (SAFE_GET_CELL(field, x, y+i) == 'f') break;
        if (SAFE_GET_CELL(field, x, y+i) == 'H') break;
        if (SAFE_GET_CELL(field, x, y+i) == 'S') {
            SAFE_SET_CELL(field, x, y+i, '.');
            break;
        }
        SAFE_SET_CELL(field, x, y+i, 'X');
        Explosion* explo = create_explosion(x, y+i, bomb);
        add_explosion(game, explo);
    }

    for (int i = 1; i < r; i++) {
        if (SAFE_GET_CELL(field, x, y-i) == 'f') break;
        if (SAFE_GET_CELL(field, x, y-i) == 'H') break;
        if (SAFE_GET_CELL(field, x, y-i) == 'S') {
            SAFE_SET_CELL(field, x, y-i, '.');
            break;
        }
        SAFE_SET_CELL(field, x, y-i, 'X');
        Explosion* explo = create_explosion(x, y-i, bomb);
        add_explosion(game, explo);
    }

    while (servermessages->nextmsg != NULL) {
        servermessages = servermessages->nextmsg;
    }

    ServerMessage* next = malloc(sizeof(ServerMessage));
    next->nextmsg = NULL;
    servermessages->nextmsg = (struct ServerMessage*)next;

    Message tx_msg = (Message){
        .type = MSG_EXPLOSION_START,
        .sender_id = 255,
        .target_id = 254,
        .data.explosion = {
            .radius = r - 1,
            .position = cell_to_uint(field, x, y),
        }
    };

    servermessages->has_content = 1;
    servermessages->msg = tx_msg;

    print_playingField(field);

}


int explode_antibomb(GameState* game, Antibomb* antibomb, ServerMessage* servermessages) {

    printf("GAME INFO: Trying to explode bomb....");

    PlayingField* field = &game->wallmap;
    uint8_t x = antibomb->x;
    uint8_t y = antibomb->y;
    uint8_t r = antibomb->radius + 1;
    uint8_t lifetime = antibomb->lifetime;

    while (servermessages->nextmsg != NULL) {
        servermessages = servermessages->nextmsg;
    }

    ServerMessage* next = malloc(sizeof(ServerMessage));
    next->nextmsg = NULL;
    servermessages->nextmsg = (struct ServerMessage*)next;

    Message tx_msg = (Message){
        .type = MSG_EXPLOSION_END,
        .sender_id = 255,
        .target_id = 254,
        .data.explosion = {
            .radius = antibomb->radius,
            .position = cell_to_uint(field, x, y),
        }
    };

    servermessages->has_content = 1;
    servermessages->msg = tx_msg;

    print_playingField(field);

}


Explosion* create_explosion(uint8_t x, uint8_t y, Bomb* bomb) {
    Explosion* explo = malloc(sizeof(Explosion));
    explo->x = x;
    explo->y = y;
    explo->lifetime = bomb->lifetime;
    return explo;
}

void add_explosion(GameState* game, Explosion* explosion) {
    Explosion* currentexplo = game->explosions;

    if (currentexplo == NULL) {
        game->explosions = explosion;
        return;
    }

    while (currentexplo->nextexplo != NULL) {
        currentexplo = currentexplo->nextexplo;
    }

    currentexplo->nextexplo = explosion;
}


void process_explosions(GameState* game) {
    if (game->explosions == NULL) {
        return;
    }

    while (game->explosions->lifetime <= 1) {
        Explosion* nextexplosion = game->explosions->nextexplo;
        clear_explosion_from_map(&game->wallmap, game->explosions->x, game->explosions->y);
        free(game->explosions);
        game->explosions = nextexplosion;

        if (game->explosions == NULL) return;
    }

    game->explosions->lifetime -= 1;

    Explosion* currentexplosion = game->explosions->nextexplo;
    Explosion* prevexplosion = game->explosions;

    while (currentexplosion != NULL) {
        if (currentexplosion->lifetime <= 1) {
            Explosion* nextexplosion = currentexplosion->nextexplo;
            prevexplosion->nextexplo = nextexplosion;
            clear_explosion_from_map(&game->wallmap, game->explosions->x, game->explosions->y);
            free(currentexplosion);
            currentexplosion = nextexplosion;
        } else {
            currentexplosion->lifetime -= 1;
            currentexplosion = currentexplosion->nextexplo;
        }
    }

}


void process_bombs(GameState* game, ServerMessage* servermessages) {
    if (game->bombs == NULL) {
        return;
    }

    while (game->bombs->lifetime <= 1) {
        Bomb* nextbomb = game->bombs->nextbomb;
        explode_bomb(game, game->bombs, servermessages);
        free(game->bombs);
        game->bombs = nextbomb;

        if (game->bombs == NULL) return;
    }

    game->bombs->lifetime -= 1;

    Bomb* currentbomb = game->bombs->nextbomb;
    Bomb* prevbomb = game->bombs;

    while (currentbomb != NULL) {
        if (currentbomb->lifetime <= 1) {
            Bomb* nextbomb = currentbomb->nextbomb;
            prevbomb->nextbomb = nextbomb;
            explode_bomb(game, currentbomb, servermessages);
            free(currentbomb);
            currentbomb = nextbomb;
        } else {
            currentbomb->lifetime -= 1;
            currentbomb = currentbomb->nextbomb;
        }
    }

}


void process_antibombs(GameState* game, ServerMessage* servermessages) {
    if (game->antibombs == NULL) {
        return;
    }

    while (game->antibombs->lifetime <= 1) {
        Antibomb* nextantibomb = game->antibombs->nextantibomb;
        explode_antibomb(game, game->antibombs, servermessages);
        free(game->antibombs);
        game->antibombs = nextantibomb;

        if (game->antibombs == NULL) return;
    }

    game->antibombs->lifetime -= 1;

    Antibomb* currentantibomb = game->antibombs->nextantibomb;
    Antibomb* prevantibomb = game->antibombs;

    while (currentantibomb != NULL) {
        if (currentantibomb->lifetime <= 1) {
            Antibomb* nextantibomb = currentantibomb->nextantibomb;
            prevantibomb->nextantibomb = nextantibomb;
            explode_antibomb(game, currentantibomb, servermessages);
            free(currentantibomb);
            currentantibomb = nextantibomb;
        } else {
            currentantibomb->lifetime -= 1;
            currentantibomb = currentantibomb->nextantibomb;
        }
    }

}


void add_bomb(GameState* game, Bomb* bomb) {
    Bomb* currentbomb = game->bombs;

    if (currentbomb == NULL) {
        game->bombs = bomb;
        return;
    }


    while (currentbomb->nextbomb != NULL) {
        currentbomb = currentbomb->nextbomb;
    }


    currentbomb->nextbomb = bomb;

}


int create_bomb(GameState* game, uint8_t target_x, uint8_t target_y, Player* p) {

    Bomb* bomb = malloc(sizeof(Bomb));

    bomb->x = target_x;
    bomb->y = target_y;
    bomb->lifetime = DEFAULT_BOMB_TIMER;
    bomb->radius = 1 + p->p_size;

    add_bomb(game, bomb);
    SAFE_SET_CELL(&game->wallmap, target_x, target_y, 'B');
}


int clear_explosion_from_map(PlayingField* field, uint8_t x, uint8_t y) {
    if (SAFE_GET_CELL(field, x, y) != 'X') {
        return -1;
    }

    SAFE_SET_CELL(field, x, y, '.');
}





