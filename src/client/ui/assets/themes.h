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
};

const struct ThemedSprites* get_current_theme();
