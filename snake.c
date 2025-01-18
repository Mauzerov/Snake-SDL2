#include "snake.h"
#include "image.h"
#include "config.h"

#include <assert.h>

/**
RRRRR  EEEEE NN   N DDDDD  EEEEE RRRRR  
RR  RR EE    NNN  N DD  DD EE    RR  RR 
RRRRR  EEEE  N NN N DD  DD EEEE  RRRRR  
RR  RR EE    N  NNN DD  DD EE    RR  RR 
RR  RR EEEEE N   NN DDDDD  EEEEE RR  RR
**/

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
        (*snake)[i].x = INITIAL_SNAKE_X + ((long)size - i - 1);
        (*snake)[i].y = INITIAL_SNAKE_Y;
    }
}

/*
 CCCC   OOOOO  LL    LL    IIII  SSSSS IIII  OOOOO  NN   N  SSSSS 
CC  CC OO   OO LL    LL     II  SS      II  OO   OO NNN  N SS     
CC     OO   OO LL    LL     II   SSSS   II  OO   OO N NN N  SSSS  
CC  CC OO   OO LL    LL     II      SS  II  OO   OO N  NNN     SS 
 CCCC   OOOOO  LLLLL LLLLL IIII SSSSS  IIII  OOOOO  N   NN SSSSS  
*/

void rotate90(int * dx, int * dy) {
    int tmp = *dx;
    *dx = -*dy, *dy = tmp;
}

#define rotate180(dx, dy) do {  \
    rotate90(dx, dy);           \
    rotate90(dx, dy);           \
} while (0)

void snake_resize(Entity ** snake, int snake_size) {
    *snake = realloc(*snake, ((snake_size)) * sizeof(Entity));
}

void handle_collectibles(Game * game, Entity ** snake, int * snake_size) {
    if (is_overlapping(*snake, &(game->apple))) {
        int apple_action_index = save_rand(game) % APLLE_ACTIONS;
        game->score += APPLE_SCORE;
        game->apple_actions[apple_action_index](game);
        game->apple_cooldown = 0;
    } else if (game->apple.y == UNDEFINED_POS && random_chance(game, APPLE_SHOW_CHANCE)) {
        game->apple_cooldown = APPLE_INTERVAL;
        game->ongoing = random_position(game, &(game->apple));
    }
    if (is_overlapping(*snake, &(game->berry))) {
        snake_resize(snake, *snake_size += 1);
        game->score += BERRY_SCORE;
        game->ongoing = random_position(game, &(game->berry));
    }
}

bool handle_porters(Game * game, Entity ** snake) {
    for (int i = 0; i < PORTER_COUNT * 2; i++) {
        if (is_overlapping(*snake, &game->porters[i])) {
            Porter * porter = &game->porters[i];
            (*snake)->x = porter->destination->x;
            (*snake)->y = porter->destination->y;
            return true;
        }
    }
    return false;
}

bool is_head_overlaping(Entity * snake, int headx, int heady, int snake_size) {
    Point point = { headx, heady };
    for (int i = 1; i < snake_size; i++) {
        if (is_overlapping(&point, &snake[i]))
            return true;
    }
    return false;
}

/*
MM   MM  OOOOO  V    V EEEEE MM   MM EEEEE NN   N TTTTTT 
MMM MMM OO   OO VV  VV EE    MMM MMM EE    NNN  N TTTTTT 
M MMM M OO   OO VV  VV EEEE  M MMM M EEEE  N NN N   TT   
M  M  M OO   OO  VVVV  EE    M  M  M EE    N  NNN   TT   
M     M  OOOOO    VV   EEEEE M     M EEEEE N   NN   TT   
*/

void snake_move(Game * game) {
    if (!(game->delta.x != 0 || game->delta.y != 0) && "snake_move should only be called when the snek can move")
        return;

    int * snake_size = &(game->snake_size);
    Entity ** snake = &(game->snake);
    Entity * head = *snake;
    Point * delta = &game->delta;

    if (is_head_overlaping(*snake, head->x, head->y, *snake_size)) {
        game->ongoing = FINISHING;
        return;
    }

    if (is_outofbounds2(head->x + delta->x, head->y + delta->y))
        rotate90(&delta->x, &delta->y);
    // if snake is still out of bounds
    if (is_outofbounds2(head->x + delta->x, head->y + delta->y))
        rotate180(&delta->x, &delta->y);

    Entity new_head = (Entity) {
        head->x + delta->x,
        head->y + delta->y,
    };

    handle_collectibles(game, snake, snake_size);

    memmove((*snake) + 1, (*snake), sizeof(Entity) * (*snake_size - 1));
    memcpy((*snake), &new_head, sizeof(Entity));

    memcpy(&game->prev, delta, sizeof(Point));

    handle_porters(game, snake);
}