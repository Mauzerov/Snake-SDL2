#ifndef ENTITY_H
#define ENTITY_H

#include "image.h"

typedef struct Porter {
    int x, y; // indirect inheritance (Point)
    int identifier;
    struct Porter * destination;
} Porter;

typedef struct {
    int x, y; // indirect inheritance (Point)
    int animation_frame;
} Entity;

typedef struct Game Game;

void draw_entity(SDL_Renderer * renderer, Entity * entity, Image * texture);

void draw_porter(
    SDL_Renderer * renderer,
    SDL_Texture * charmap,
    Porter * porter,
    Image * textures[Texture_COUNT]
);

void draw_apple_timer(SDL_Renderer * renderer, Game * game);

#endif