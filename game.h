#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "entity.h"

typedef struct Game {
    Entity apple;
    Entity berry;
    Entity * snake;
    int snake_size;
    int apple_timer;
    int score;
    struct tm elapsed_time;
    float time_scale;
    int seed;
    Porter porters[PORTER_COUNT * 2];
    int dx, dy;
    int ongoing;
    void (*apple_actions[2])(struct Game*);
    Image * textures[Texture_COUNT];
} Game;

#endif