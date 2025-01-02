#include <stdbool.h>

#include "config.h"
#include "game.h"

int save_rand(Game * game) {
    return game->seed = rand();
}

bool _random_position(Game * game, Point * entity) {
    bool valid = false;
    if (game->snake_size > GAME_SIZE * GAME_SIZE)
        return valid;
    do {
        valid = true;
        entity->x = save_rand(game) % GAME_SIZE;
        entity->y = save_rand(game) % GAME_SIZE;

        for (int i = 0; i < game->snake_size; i++) {
            if (is_overlapping(&(game->snake[i]), entity)) {
                valid = false;
                break;
            }
        }

        for (int i = 0; i < PORTER_COUNT * 2; i++) {
            if ((Point *)&game->porters[i] != entity
                    && is_overlapping(entity, &game->porters[i])) {
                valid = false;
                break;
            }
        }
    } while (!valid);
    return valid;
}


void handle_outofbounds(Game * game) {
    int *dx = &game->dx, *dy = &game->dy;
    // undo move
    game->snake[0].x -= *dx;
    game->snake[0].y -= *dy;

    if        (*dx == -1) { // moving left -> move up
        *dy = *dx;
        *dx = 0;
    } else if (*dx == +1) { // moving right -> move down
        *dy = *dx;
        *dx = 0;
    } else if (*dy == -1) { // moving up -> move right
        *dx = -*dy;
        *dy = 0;
    } else if (*dy == +1) { // moving down -> move left
        *dx = -*dy;
        *dy = 0;
    }

    game->snake[0].x += *dx;
    game->snake[0].y += *dy;

    if (is_outofbounds(&game->snake[0])) {
        // rotate movement
        game->snake[0].x += (*dx *= -1) * 2;
        game->snake[0].y += (*dy *= -1) * 2;
    }
}
