#include "snake.h"
#include "image.h"
#include "config.h"

#include <assert.h>

Direction _get_direction(Point * prev, Point * curr) {
    if (prev->x + 1 == curr->x && prev->y == curr->y)
        return Right;

    if (prev->x - 1 == curr->x && prev->y == curr->y)
        return Left;

    if (prev->y - 1 == curr->y && prev->x == curr->x)
        return Up;

    if (prev->y + 1 == curr->y && prev->x == curr->x)
        return Down;

    return Unknown;
}

void render_snake(
    SDL_Renderer * renderer,
    Entity * snake,
    size_t size,
    Image * textures[Texture_COUNT]
) {
    assert(size >= 3 && "Snake has to have a body!");

    size_t last = size - 1;

    SDL_Rect rect = {
        snake[last].x * TILE_SIZE, snake[last].y * TILE_SIZE, 
        TILE_SIZE, TILE_SIZE
    };

    // Render Tail
    render_square_image(
        renderer,
        textures[Texture_TAIL],
        &rect, last,
        get_direction(&snake[last], &snake[last - 1])
    );

    // Render snake body
    for (size_t i = 0; i <= last; i++) {
        Entity * curr = &snake[i];

        rect.x = curr->x * TILE_SIZE;
        rect.y = curr->y * TILE_SIZE;

        for (int di = -1; di <= 1; di += 2) {
            if (0 > (int)i + di || i + di >= size)
                continue;
            render_square_image(
                renderer,
                textures[ (i == 0) ? Texture_HEAD : Texture_BODY ],
                &rect, i,
                get_direction(&snake[i + di], curr)
            );
        }
    }
}