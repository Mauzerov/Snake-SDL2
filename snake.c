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

void handle_collectibles(Game * game, Entity * head, Entity ** snake, int * snake_size) {
    if (is_overlapping(head, &(game->apple))) {
        int apple_action_index = save_rand(game) % 2;
        game->score += APPLE_SCORE;
        game->apple_actions[apple_action_index](game);
        game->apple_timer = 0;
        game->apple.x = game->apple.y = UNDEFINED_POS;
    } else if (game->apple_timer <= -1 && game->apple.y != UNDEFINED_POS) {
        game->apple.x = game->apple.y = UNDEFINED_POS;
    } else if (game->apple.y == UNDEFINED_POS && random_chance(game, APPLE_SHOW_CHANCE)) {
        game->apple_timer = APPLE_TIMER_CAP * FRAMES_PER_SECOND;
        game->ongoing = random_position(game, &(game->apple));
    }
    if (is_overlapping(head, &(game->berry))) {
        snake_resize(snake, *snake_size += 1);
        game->score += BERRY_SCORE;
        game->ongoing = random_position(game, &(game->berry));
    }
}

void handle_porters(Game * game, Entity ** snake) {
    for (int i = 0; i < PORTER_COUNT * 2; i++) {
        if (is_overlapping(*snake, &game->porters[i])) {
            Porter * porter = &game->porters[i];
            (*snake)->x = porter->destination->x;
            (*snake)->y = porter->destination->y;
            break;
        }
    }
}

bool is_snake_overlaping(Entity * snake, int headx, int heady, int snake_size) {
    Point point = { headx, heady };
    for (int i = 1; i < snake_size; i++) {
        if (is_overlapping(&point, &snake[i]))
            return true;
    }
    return false;
}

void snake_move(Game * game) {
    if (!(game->dx != 0 || game->dy != 0) && "snake_move should only be called when the snek can move")
        return;

    int * snake_size = &(game->snake_size);
    Entity ** snake = &(game->snake);

    Entity * head = *snake;
    if (is_snake_overlaping(*snake, head->x, head->y, *snake_size)) {
        game->ongoing = FINISHING;
        return;
    }

    if (is_outofbounds2(head->x + game->dx, head->y + game->dy))
        rotate90(&game->dx, &game->dy);
    if (is_outofbounds2(head->x + game->dx, head->y + game->dy)) {
        rotate90(&game->dx, &game->dy);
        rotate90(&game->dx, &game->dy);
    }

    Entity new_head = (Entity) {
        head->x + game->dx,
        head->y + game->dy,
        ANIMATION_SIZE
    };

    handle_collectibles(game, &new_head, snake, snake_size);

    memmove((*snake) + 1, (*snake), sizeof(Entity) * (*snake_size - 1));
    memcpy((*snake), &new_head, sizeof(Entity));

    handle_porters(game, snake);
}