#include "engine.h"
#include "config.h"
#include "image.h"

int SDL_RenderText(
    SDL_Renderer * renderer,
    SDL_Texture * charmap,
    const char * string,
    SDL_Color fg,
    int x, int y, int out_width
) {
    float scale = out_width / (float) CHAR_WIDTH;
    SDL_Rect src_rect = { 0, 0, CHAR_WIDTH, CHAR_WIDTH },
             dst_rect = { x, y, CHAR_WIDTH * scale, CHAR_WIDTH * scale };

    SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, 255);
    // TODO: implement proper color scaling
    //       above code doesn't do anything

    for (int c; (c = (int)*(string++) & CHARMAP_MASK) != '\0'; ) {
        if ((char)c == '\n') {
            dst_rect.x = x;
            dst_rect.y += out_width;
            continue;
        }
        src_rect.x = (c % CHARMAP_SIZE) * CHAR_WIDTH;
        src_rect.y = (c / CHARMAP_SIZE) * CHAR_WIDTH;
        SDL_RenderCopy(renderer, charmap, &src_rect, &dst_rect);
        dst_rect.x += CHAR_WIDTH * scale;
    }
    return 0;
}