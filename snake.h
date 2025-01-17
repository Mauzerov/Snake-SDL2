#ifndef SNAKE_H
#define SNAKE_H

#include <SDL2/SDL_render.h>
#include "image.h"
#include "point.h"
#include "entity.h"
#include "game.h"

void render_snake(
    SDL_Renderer * renderer,
    Entity * snake,
    long size,
    Image * textures[Texture_COUNT]
);

void snake_init(Entity ** snake, size_t size);

void snake_resize(Entity ** snake, int snake_size);

void snake_move(Game * game);

#endif