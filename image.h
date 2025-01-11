#ifndef IMAGE_H
#define IMAGE_H

#include "point.h"

#include <SDL2/SDL_render.h>

typedef enum {
    Texture_TAIL = 0,
    Texture_BODY,
    Texture_HEAD,
    Texture_APPLE,
    Texture_BERRY,
    Texture_COUNT,
} SnakeTexture;

typedef struct {
    int width, height;
    SDL_Texture * texture;
} Image;

void render_square_image(
    SDL_Renderer * renderer,
    Image * image,
    SDL_Rect * rect,
    int animation_frame,
    Direction direction
);

SDL_Texture * create_texture(
    SDL_Renderer * renderer,
    char * path
);

Image * create_image(
    SDL_Renderer * renderer,
    SDL_Texture * texture,   // loaded bitmap (with removed black)
    SDL_Rect  rect,          // which part of bitmap to crop out
    SDL_Color color          // color to map white to
);

void destroy_image(Image * image);

#endif