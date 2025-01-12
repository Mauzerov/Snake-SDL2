#include "entity.h"
#include "renderer.h"
#include "game.h"
#include "image.h"

void draw_entity(SDL_Renderer * renderer, Entity * entity, Image * texture) {
    SDL_Rect fill_rect = (SDL_Rect) {
        entity->x * TILE_SIZE,
        entity->y * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE,
    };

    render_square_image(
        renderer,
        texture,
        &fill_rect,
        entity->animation_frame,
        0
    );
}

void draw_porter(
    SDL_Renderer * renderer,
    SDL_Texture * charmap,
    Game * game,
    Porter * porter
) {
    SDL_Color fg = Color_FOREGROUND;
    char string[2] = { (char)porter->identifier, 0 };

    SDL_Rect rect = (SDL_Rect) {
        porter->x * TILE_SIZE,
        porter->y * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    };

    render_square_image(
        renderer,
        game->textures[Texture_PORTER],
        &rect, 0, 0
    );

    int index = (TILE_SIZE >> 1);
    SDL_RenderText(
        renderer, charmap, 
        string, fg,
        rect.x + (TILE_SIZE - index),
        rect.y + (TILE_SIZE - index),
        index
    );
}

void draw_apple_timer(SDL_Renderer * renderer, Game * game) {
    SDL_Color color = Color_APPLE;
    SDL_Color bg = Color_BLACK;

    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);

    SDL_Rect rect = (SDL_Rect) {
        .x = GAME_WIDTH,
        .y = 0,
        .w = APPLE_TIMER_WIDTH,
        .h = GAME_WIDTH,
    };
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

    rect.h = GAME_WIDTH / (
        (APPLE_TIMER_CAP * FRAMES_PER_SECOND) / (float)game->apple_timer
    );

    if (game->dx || game->dy)
        game->apple_timer--;

    SDL_RenderFillRect(renderer, &rect);
}
