#include "snake.h"
#include "image.h"

#include <assert.h>

//                              curr,         next
Direction get_direction(Point * prev, Point * curr) {
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
    Snake * snake
) {
    assert(snake->size >= 3 && "Snake has to have a body!");

    size_t last = snake->size - 1;

    SDL_Rect rect = { snake->body[last].x * 32, snake->body[last].y * 32, 32, 32 };

    // Render Tail
    render_square_image(
        renderer,
        snake->textures[SNAKE_TAIL],
        &rect, 0,
        get_direction(&snake->body[last], &snake->body[last - 1])
    );

    // Render snake body
    for (size_t i = 0; i < snake->size - 1; i++) {
        Point * curr = &snake->body[i + 0];
        rect.x = curr->x * 32, rect.y = curr->y * 32;

        for (int di = -1; di <= 1; di += 2) {
            if (0 > i + di || i + di >= snake->size)
                continue;
            render_square_image(
                renderer,
                snake->textures[
                    (i == 0) ? SNAKE_HEAD : SNAKE_BODY
                ],
                &rect, i,
                get_direction(&snake->body[i + di], curr)
            );
        }
    }
}