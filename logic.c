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
