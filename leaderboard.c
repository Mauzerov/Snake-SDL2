#include <string.h>


#include <SDL2/SDL_log.h>
#include "text.h"
#include "game.h"


int order_players(const void * a, const void * b) {
    return  ((Player *)b)->score - ((Player *)a)->score;
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
    while (fscanf(file, "%lu %" STR(MAX_NAME_SIZE) "[^\n]",
        &leaderboard[leaderboard_count].score,
        leaderboard [leaderboard_count].name
    ) == 2 && leaderboard_count < LEADERBOARD_SIZE)
        leaderboard_count++;
    fclose(file);

    if (leaderboard_count > 1)
        qsort(leaderboard, leaderboard_count, sizeof(Player), order_players);
    return leaderboard_count;
}

bool can_add_to_leaderboard(Game * game) {
    int records = read_leaderboard(game->leaderboard);
    for (int i = 0; i < records; i++) {
        if (game->leaderboard[i].score < game->score)
            return true;
    }
    return records != LEADERBOARD_SIZE;
}

void add_player_to_leaderboard(
    const char * name,
    size_t string_size,
    Game * game
) {
    int i, leaderboard_count = read_leaderboard(game->leaderboard);

    // assume players are sorted from lowest to highest point
    for (i = 0; i < leaderboard_count; i++) {
        if (game->score > game->leaderboard[i].score)
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
    strncpy(game->leaderboard[i].name, name, string_size);
    memset(game->leaderboard[i].name + string_size, 0, MAX_NAME_SIZE - string_size + 1);
    game->leaderboard[i].score = game->score;

    write_leaderboard(game->leaderboard);
}

char * leaderboard_to_string(Game * game, char * buffer) {
    char line[MAX_STRING_BUFFER_SIZE] = { 0 };
    for (int i = 0; i < game->records; i++) {
        sprintf(line, "%d. %-*s: %04lu\n",
            i + 1, MAX_NAME_SIZE,
            game->leaderboard[i].name,
            game->leaderboard[i].score
        );
        strcat(buffer, line);
    }
    for (int i = game->records; i < LEADERBOARD_SIZE; i++)
        strcat(buffer, "\n");
    return buffer;
}

void render_end_screen(SDL_Renderer * renderer, SDL_Texture * charmap, Game * game) {
    SDL_Color bg = Color_BLACK;
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);

    SDL_Rect rect = { 
        0, GAME_WIDTH >> 2, 
        GAME_WIDTH, GAME_WIDTH >> 1
    };
    SDL_RenderFillRect(renderer, &rect);

    char string[MAX_STRING_BUFFER_SIZE] = { 0 };
    char leaderboard[LEADERBOARD_SIZE * MAX_NAME_SIZE * 2] = { 0 };

    sprintf(
        string,
        "GAME OVER\n\n"
        "Press 'n' to start a new game!\n"
        "Press 'ESC' to quit!\n\n"
        "Your Score: %04lu\n"
        "Leaderboard:\n"
        "%s\n"
        "%s %-*s",
        game->score,
        leaderboard_to_string(game, leaderboard),
        game->text_entered ? "" : "Your Name:",
        MAX_NAME_SIZE,
        game->text_entered ? "" : game->buffer
    );

    char * centered_string = center_string(string, GAME_WIDTH / FONT_SIZE);
    SDL_RenderText(
        renderer, charmap, centered_string, Color_FOREGROUND,
        0, rect.y + 2 * FONT_SIZE, FONT_SIZE
    );
    free(centered_string);

    SDL_RenderPresent(renderer);
}