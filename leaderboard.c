#include <string.h>

#include "engine.h"
#include "game.h"


int order_players(const void * a, const void * b) {
    return ((Player *)a)->score - ((Player *)b)->score;
}

void write_leaderboard(Player leaderboard[LEADERBOARD_SIZE]) {
    FILE * file = fopen(LEADERBOARD_FILENAME, "w");
    if (file == NULL)
        return;
    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        if (strlen(leaderboard[i].name) == 0)
            break;
        fprintf(file, "%lu %s\n",
            leaderboard[i].score,
            leaderboard[i].name
        );
    }
    fclose(file);
}

int read_leaderboard(Player leaderboard[LEADERBOARD_SIZE]) {
    FILE * file = fopen(LEADERBOARD_FILENAME, "r");

    if (file == NULL)
        return 0;

    int leaderboard_count = 0;
    while (fscanf(file, "%lu %19[^\n]",
        &leaderboard[leaderboard_count].score,
        leaderboard [leaderboard_count].name
    ) == 2 && leaderboard_count < LEADERBOARD_SIZE)
        leaderboard_count++;
    fclose(file);

    if (leaderboard_count > 1)
        qsort(leaderboard, leaderboard_count, sizeof(Player), order_players);
    return leaderboard_count;
}

void add_player_to_leaderboard(
    const char * name,
    Game * game
) {
    // TODO: to allow continuous play change this to game->leaderboard
    int i, leaderboard_count = read_leaderboard(game->leaderboard);

    // assume players are sorted from lowest to highest point
    for (i = 0; i < leaderboard_count; i++) {
        if (game->score < game->leaderboard[i].score)
            break; // insert player at (i)
    }

    if (i == LEADERBOARD_SIZE)
        return; // current player scored the highest

    // move remaining players back
    memmove(
        game->leaderboard + i + 1, game->leaderboard + i,
        sizeof (Player) * (LEADERBOARD_SIZE - i - 1)
    );
    // set leaderboard data
    strcpy(game->leaderboard[i].name, name);
    game->leaderboard[i].score = game->score;

    write_leaderboard(game->leaderboard);
}

void read_player_name(struct Game * game) {
    char name[MAX_NAME_SIZE] = { 0 };
    size_t string_size = 0;
    bool name_entered = false;

    SDL_Event e;

    while (!name_entered) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                exit(0);
            case SDL_KEYDOWN: {
                int key = e.key.keysym.sym;

                if (key == SDLK_RETURN) {
                    name_entered = true;
                } else if (key == SDLK_BACKSPACE && string_size > 0) {
                    name[string_size--] = 0;
                } else if (isprint(key)) {
                    name[string_size++] = (char)key;
                }
            } break;
            }
        }
    }
    
    if (strlen(name) > 0) {
        add_player_to_leaderboard(name, game);
    }
}