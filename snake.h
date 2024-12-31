#ifndef SNAKE_H
#define SNAKE_H

#include <SDL2/SDL.h>
#include "image.h"
#include "point.h"

typedef enum {
    SNAKE_TAIL = 0,
    SNAKE_BODY,
    SNAKE_HEAD,
    SNAKE_TEXTURE_COUNT,
} SnakeTexture;

typedef struct {
    size_t size;
    Point * body;
    Image * textures[SNAKE_TEXTURE_COUNT]; // tail, body, head
} Snake;

Direction get_direction(Point * a, Point * b);

void render_snake(
    SDL_Renderer * renderer,
    Snake * snake
);

#endif