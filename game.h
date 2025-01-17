#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <time.h>

#include "config.h"
#include "image.h"
#include "entity.h"

#include <SDL2/SDL_events.h>

typedef struct {
    char name[MAX_NAME_SIZE + 1];
    unsigned long score;
} Player;

typedef struct Game {
    Entity apple;
    Entity berry;
    Entity * snake;
    int snake_size;
    float apple_cooldown,
          elapsed_time,
          move_cooldown,
          animation_cooldown,
          speedup_cooldown,
          game_speed_scale;
    unsigned long score;
    int seed;
    Porter porters[PORTER_COUNT * 2];
    int dx, dy;
    int ongoing;
    char buffer[MAX_NAME_SIZE + 1];
    size_t buffer_count;
    bool text_entered;
    void (*apple_actions[2])(struct Game*);
    Player leaderboard[LEADERBOARD_SIZE];
    int records;
} Game;

#define _is_outofbounds(x, y) \
    ((x) < 0 || x >= GAME_SIZE || (y) < 0 || (y) >= GAME_SIZE)
#define is_outofbounds(a) _is_outofbounds((a)->x, (a)->y)
#define is_outofbounds2(x, y) _is_outofbounds(x, y)

#define _is_overlapping(a, b) \
    ((a) != (b) &&            \
    ((a)->x == (b)->x && (a)->y == (b)->y))
#define is_overlapping(a, b) _is_overlapping((Point*)a, (Point*)b)

int save_rand(Game * game);

bool _random_position(Game * game, Point * entity);

#define random_position(game, object) _random_position(game, (Point*)object)

#define random_chance(game, chance) ((float)(save_rand(game) % 10000) / 100.f <= (float)chance)

void porters_init(Game * game);

void save_game(Game * game);

void load_game(Game * game);

void new_game(Game * game);

void handle_outofbounds(Game * game);

void load_game_textures(
    SDL_Renderer * renderer,
    SDL_Texture * texture,
    Image * textures[Texture_COUNT]
);

void destroy_game_textures(Image * textures[Texture_COUNT]);

bool can_add_to_leaderboard(Game *);

void add_player_to_leaderboard(const char *, size_t, Game *);

void read_player_name(Game *);

int read_leaderboard(Player[LEADERBOARD_SIZE]);

void write_leaderboard(Player[LEADERBOARD_SIZE]);

int order_players(const void *, const void *);

void render_game(
    SDL_Renderer * renderer,
    Game * game,
    SDL_Texture * charmap,
    Image * textures[Texture_COUNT]
);

void render_game_info(SDL_Renderer * renderer, Game * game, SDL_Texture * charmap);

void render_end_screen(SDL_Renderer * renderer, SDL_Texture * charmap, Game * game);

void apple_action_slowdown(Game * game);

void apple_action_shorten(Game * game);

#endif