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
    long size,
    Image * textures[Texture_COUNT]
) {
    assert(size >= 3 && "Snake has to have a body!");

    long last = size - 1;

    SDL_Rect rect = {
        snake[last].x * TILE_SIZE, snake[last].y * TILE_SIZE, 
        TILE_SIZE, TILE_SIZE
    };

    // Render snake body
    for (long i = last; i >= 0; i--) {
        Entity * curr = &snake[i];

        rect.x = curr->x * TILE_SIZE;
        rect.y = curr->y * TILE_SIZE;

        for (long di = -1; di <= 1; di += 2) {
            if (0 > i + di || i + di >= size)
                continue;
            render_square_image(
                renderer,
                textures[
                    (i == 0)    ? Texture_HEAD :
                    (i == last) ? Texture_TAIL : Texture_BODY ],
                &rect, i,
                get_direction(&snake[i + di], curr)
            );
        }
    }
}

void snake_init(Entity ** snake, size_t size) {
    // if snake is not initilized
    // then malloc it
    // if snake is already initialized we can ignore it
    //    due to the fact that next realloc call will truncate the size
    if (*snake == NULL)
        *snake = malloc(sizeof(Entity) * size);

    for (size_t i = 0; i < size; i++) {
        (*snake)[i].x = INITIAL_SNAKE_X + (size - i - 1);
        (*snake)[i].y = INITIAL_SNAKE_Y;
        (*snake)[i].color = Color_SNAKE_TAIL;
    }
    (*snake)[0].color = Color_SNAKE_HEAD;
}
