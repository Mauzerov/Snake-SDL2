#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL_render.h>

#define STR2(x) #x
#define STR(X) STR2(X)

void SDL_RenderText(
    SDL_Renderer * renderer,
    SDL_Texture * charmap,
    const char * string,
    SDL_Color fg,
    int x, int y, int out_width
);

char * center_string(const char * string, int out_width);

#endif