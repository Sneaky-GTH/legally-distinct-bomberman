#pragma once
#include "../../config/config.h"
#include "./sprites.h"

struct ThemedExplosionSprites {
    SpriteId ulr;
    SpriteId udl;
    SpriteId ud;
    SpriteId udr;
    SpriteId none;
    SpriteId lr;
    SpriteId dr;
    SpriteId dl;
    SpriteId d;
    SpriteId u;
    SpriteId dlr;
    SpriteId ur;
    SpriteId ul;
    SpriteId l;
    SpriteId r;
};

struct ThemedGameBackgroundSprites {
    SpriteId bgTL;
    SpriteId bgTM;
    SpriteId bgTR;
    SpriteId bgHL;
    SpriteId bgHM;
    SpriteId bgHR;
    SpriteId bgML;
    SpriteId bgMM;
    SpriteId bgMR;
    SpriteId bgLL;
    SpriteId bgLM;
    SpriteId bgLR;
    SpriteId bgBL;
    SpriteId bgBM;
    SpriteId bgBR;
};

struct ThemedSprites {
    SpriteId top;
    SpriteId middle;
    SpriteId broken1;
    SpriteId broken2;
    SpriteId bomb;
    SpriteId background1;
    SpriteId background12;
    SpriteId background2;
    SpriteId background23;
    SpriteId background3;
    SpriteId background34;
    SpriteId background4;
    struct ThemedExplosionSprites explosion;
    struct ThemedGameBackgroundSprites game_bg;
};

const struct ThemedSprites* get_current_theme();
