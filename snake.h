#ifndef SNAKE_H
#define SNAKE_H

#include <SDL2/SDL.h>
#include "image.h"
#include "point.h"
#include "entity.h"

typedef enum {
    Texture_TAIL = 0,
    Texture_BODY,
    Texture_HEAD,
    Texture_APPLE,
    Texture_BERRY,
    Texture_COUNT,
} SnakeTexture;

typedef struct {
    size_t size;
    Point * body;
    // Image * textures[Texture_COUNT]; // tail, body, head
} Snake;

void render_snake(
    SDL_Renderer * renderer,
    Entity * snake,
    size_t size,
    Image * textures[Texture_COUNT]
);

#endif