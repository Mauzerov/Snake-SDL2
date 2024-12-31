#ifndef IMAGE_H
#define IMAGE_H

#include <SDL2/SDL.h>

#define ALTERNATE_SIZES 2

typedef struct {
    int width, height;
    SDL_Texture * texture;
} Image;

void render_square_image(
    SDL_Renderer * renderer,
    Image * image,
    SDL_Rect * rect,
    int animation_frame
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

#endif