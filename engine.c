#include "renderer.h"
#include "config.h"
#include "image.h"

void SDL_RenderText(
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
}

char * center_string(const char *input, int width) {
    char *output = (char *)calloc(MAX_STRING_BUFFER_SIZE, sizeof(char));
    char *output_ptr = output;

    for (
        const char *line_start = input, *line_end = NULL;
        *line_start != '\0';
        line_start = line_end + 1
    ) {
        line_end = strchr(line_start, '\n');
        if (!line_end) {
            line_end = line_start + strlen(line_start);
        }

        int line_length = line_end - line_start;

        int formatted_size = sprintf(
            output_ptr,
            "%*s%.*s\n",
            (width - line_length) >> 1, "",
            line_length,
            line_start
        );
        output_ptr += formatted_size;
    }

    return output;
}