#ifndef SDL_EXTENSION
#define SDL_EXTENSION

#include <SDL2/SDL_render.h>

int SDL_RenderText(
    SDL_Renderer * renderer,
    SDL_Texture * charmap,
    const char * string,
    SDL_Color fg,
    int x, int y, int out_width
);

#endif