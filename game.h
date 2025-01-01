#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <time.h>

#include "config.h"
#include "image.h"
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


#define _is_outofbounds(a) \
    ((a)->x < 0 || (a)->x >= GAME_SIZE || (a)->y < 0 || (a)->y >= GAME_SIZE)
#define is_outofbounds(a) _is_outofbounds((Point*)a)

#define _is_overlapping(a, b) \
    ((a)->x == (b)->x && (a)->y == (b)->y)
#define is_overlapping(a, b) _is_overlapping((Point*)a, (Point*)b)

int save_rand(Game * game);

bool _random_position(Game * game, Point * entity);

#define random_position(game, object) _random_position(game, (Point*)object)

#define save_file_operattion(file_fn, file, g, game) do {   \
    file_fn(file, "%d %d\n", g->apple.x, g->apple.y);       \
    file_fn(file, "%d %d\n", g->berry.x, g->berry.y);       \
    file_fn(file, "%d\n", g->snake_size);                   \
    /* note that in the for loop `game` is used */          \
    for (int i = 0; i < game->snake_size; i++) {            \
        file_fn(file, "%d %d\n",                            \
            g->snake[i].x, g->snake[i].y);                  \
    }                                                       \
    file_fn(file, "%d %d %d %f\n",                          \
        g->dx, g->dy,                                       \
        g->apple_timer, g->time_scale);                     \
    file_fn(file, "%d %d\n", g->score, g->seed);            \
    file_fn(file, "%d %d\n",                                \
        g->elapsed_time.tm_min,                             \
        g->elapsed_time.tm_sec);                            \
    for (int i = 0; i < PORTER_COUNT * 2; i++) {            \
        file_fn(file, "%d %d %d\n",                         \
            g->porters[i].x, g->porters[i].y,               \
            g->porters[i].identifier);                      \
    }                                                       \
} while (0)

void porters_init(Game * game);

void save_game(Game * game);

void load_game(Game * game);

void new_game(Game * game);

void handle_outofbounds(Game * game);

void load_game_textures(
    SDL_Renderer * renderer,
    Game * game,
    SDL_Texture * texture
);

#endif