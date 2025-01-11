#include "snake.h"
#include "image.h"
#include "config.h"

#include <assert.h>

void render_snake(
    SDL_Renderer * renderer,
    Entity * snake,
    long size,
    Image * textures[Texture_COUNT]
) {
    assert(size >= 3 && "Snake has to have a body!");

    SDL_Rect rect = {
        snake[size - 1].x * TILE_SIZE, snake[size - 1].y * TILE_SIZE, 
        TILE_SIZE, TILE_SIZE
    };

    // Render snake body backwards so that the head is on top
    for (long i = size - 1; i >= 0; i--) {
        Entity * curr = &snake[i];
        rect.x = curr->x * TILE_SIZE;
        rect.y = curr->y * TILE_SIZE;

        for (long di = -1; di <= 1; di += 2) {
            if (0 > i + di || i + di >= size)
                continue;
            render_square_image(
                renderer,
                textures[
                    (i == size -1) ? Texture_TAIL :
                    (i == 0) ? Texture_HEAD : Texture_BODY ],
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
    }
}

void rotate90(int * dx, int * dy) {
    int tmp = *dx;
    *dx = -*dy, *dy = tmp;
}

void snake_resize(Entity ** snake, int snake_size) {
    *snake = realloc(*snake, ((snake_size)) * sizeof(Entity));
}

void handle_collectibles(Game * game, Entity ** snake, int * snake_size) {
    if (is_overlapping(*snake, &(game->apple))) {
        int apple_action_index = save_rand(game) % 2;
        game->score += APPLE_SCORE;
        game->apple_actions[apple_action_index](game);
        game->apple_timer = 0;
        game->apple.x = game->apple.y = -100;
    } else if (game->apple_timer == 0) {
        game->apple.x = game->apple.y = -100;
    } else if (random_chance(game, 10)) {
        game->apple_timer = APPLE_TIMER_CAP * FRAMES_PER_SECOND;
        game->ongoing = random_position(game, &(game->apple));
    }
    if (is_overlapping(*snake, &(game->berry))) {
        snake_resize(snake, *snake_size += 1);
        game->score += BERRY_SCORE;
        game->ongoing = random_position(game, &(game->berry));
    }
}

void snake_move(Game * game) {
    assert((game->dx != 0 || game->dy != 0) && "snake_move should only be called when the snek can move");

    int * snake_size = &(game->snake_size);
    Entity ** snake = &(game->snake);

    for (int i = 1; i < *snake_size; i++) {
        if (is_overlapping(*snake, &(*snake)[i])) {
            game->ongoing = FINISHING;
            return;
        }
    }

    Entity * head = *snake;
    for (int i = 0; is_outofbounds2(head->x + game->dx, head->y + game->dy) && i < 4; i++) {
        rotate90(&game->dx, &game->dy);
    }
    int headx = head->x + game->dx,
        heady = head->y + game->dy;

    handle_collectibles(game, snake, snake_size);

    memmove((*snake) + 1, (*snake), sizeof(Entity) * (*snake_size - 1));

    (*snake)[0] = (Entity) { headx, heady, ANIMATION_SIZE };

    for (int i = 0; i < PORTER_COUNT * 2; i++) {
        if (is_overlapping(*snake, &game->porters[i])) {
            Porter * porter = &game->porters[i];
            (*snake)[0].x = porter->destination->x;
            (*snake)[0].y = porter->destination->y;
            break;
        }
    }
}