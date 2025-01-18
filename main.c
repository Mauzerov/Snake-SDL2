#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include "renderer.h"
#include "config.h"
#include "image.h"
#include "snake.h"
#include "game.h"

#include <SDL2/SDL.h>

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

        if (is_overlapping(&game->berry, entity)
        ||  is_overlapping(&game->apple, entity)) {
            continue;
        }

        for (int i = 0; i < PORTER_COUNT * 2; i++) {
            if (is_overlapping(entity, &game->porters[i])) {
                valid = false;
                break;
            }
        }
    } while (!valid);
    return valid;
}


void update_animations(Game * game) {
    static int animation_frame = 0;
    game->apple.animation_frame++;
    game->berry.animation_frame++;
    // keep the animation frame of the snake (due to movement implementation)
    game->snake[0].animation_frame = animation_frame++;
}

void handle_keyboard_event(SDL_KeyboardEvent * e, Game * game) {
    int * dx = &game->delta.x, *dy = &game->delta.y;
    int * px = &game->prev.x,  *py = &game->prev.y;
    if (e->repeat)
        return;
    switch (e->keysym.sym) {
    // Movement
    case SDLK_LEFT:  if (*px != +1) {
        *dx = -1; *dy = +0;
    }
        break;
    case SDLK_UP:    if (*py != +1) {
        *dx = +0;  *dy = -1;
    }
        break;
    case SDLK_RIGHT: if (*px != -1) {
        *dx = +1;  *dy = +0;
    }
        break;
    case SDLK_DOWN:  if (*py != -1) {
        *dx = +0;  *dy = +1;
    }
        break;
    // Utility 
    case SDLK_n: {
        new_game(game);
        break;
    }
    case SDLK_s: {
        save_game(game);
        break;
    }
    case SDLK_l: {
        load_game(game);
        break;
    }
    case SDLK_ESCAPE: {
        exit(EXIT_SUCCESS);
        break;
    }
    default:
        return;
    }
}

void handle_text_input(SDL_KeyboardEvent * e, Game * game) {
    int key = e->keysym.sym;
    if (key == SDLK_ESCAPE) {
        game->buffer[game->buffer_count = 0] = '\0';
        game->text_entered = true;
    } else if (key == SDLK_RETURN) {
        if (game->buffer_count > 0) {
            add_player_to_leaderboard(game->buffer, game->buffer_count, game);
            game->records = read_leaderboard(game->leaderboard);
            game->buffer[game->buffer_count = 0] = '\0';
        }   
        game->text_entered = true;
    } else if (key == SDLK_BACKSPACE && game->buffer_count > 0) {
        game->buffer[--game->buffer_count] = '\0';
    } else if (key >= 0 && key <= 255 && isprint(key) && game->buffer_count < MAX_NAME_SIZE) {
        game->buffer[game->buffer_count++] = (char)key;
    }
}

void handle_events(Game * game) {
    static SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            exit(EXIT_SUCCESS);

        switch (e.type) {
        case SDL_KEYDOWN:
            if (game->ongoing == FINISHED && !game->text_entered)
                handle_text_input(&e.key, game);
            else handle_keyboard_event(&e.key, game);
            break;
        default:
            continue;
        }
    }
}

#define handle_cooldown(cooldown, _default, dt, code_block) do { \
    if ((cooldown) <= 0.f) {    \
        code_block;             \
        cooldown += _default;   \
    } else cooldown -= dt;      \
} while (0)

void handle_updates(Game * game, float delta_time) {
    if (!game->delta.x && !game->delta.y)
        return;
    game->elapsed_time += delta_time;

    handle_cooldown(
        game->apple_cooldown,
        0,
        delta_time,
        game->apple.x = game->apple.y = UNDEFINED_POS
    );

    handle_cooldown(
        game->move_cooldown,
        1.f / (FRAMES_PER_SECOND * game->game_speed_scale),
        delta_time,
        snake_move(game)
    );

    handle_cooldown(
        game->animation_cooldown,
        ANIMATION_INTERVAL,
        delta_time,
        update_animations(game)
    );

    handle_cooldown(
        game->speedup_cooldown,
        SPEED_SCALE_INTERVAL,
        delta_time,
        game->game_speed_scale += TIME_SCALE_CHANGE
    );
}

int main_loop(
    SDL_Renderer * renderer,
    Game * game,
    SDL_Texture * charmap,
    Image * textures[Texture_COUNT]
) {
    unsigned long prev_ticks = SDL_GetTicks();

    while (1) {
        unsigned long long current_ticks = SDL_GetTicks();
        float delta_time = (current_ticks - prev_ticks) * 0.001f;

        handle_events(game);

        if (game->ongoing <= FINISHING) {
            if (game->ongoing == FINISHING) {
                game->ongoing = FINISHED;
                game->text_entered = !can_add_to_leaderboard(game);
            }
            render_end_screen(renderer, charmap, game);
            SDL_Delay(LAZY_DELAY);
            continue;
        }

        handle_updates(game, delta_time);

        render_game(renderer, game, charmap, textures);
        
        prev_ticks = current_ticks;

        SDL_Delay((int)(1000.f / (float)REFRESH_RATE));
    }
}

int main() {
    srand(time( NULL ));

    SDL_Window * window     = NULL;
    SDL_Renderer * renderer = NULL;
    
    Game game = (Game) {
        .apple_actions = { apple_action_shorten, apple_action_slowdown },
    };
    new_game(&game);

    if (SDL_Init ( SDL_INIT_VIDEO ) < 0) {
        fprintf(stderr, "Could Not Initialize!: %d %s\n", __LINE__, SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);
    if (window == NULL) {
        fprintf(stderr, "Could Not Initialize!: %d %s\n", __LINE__, SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture * charmap = create_texture(renderer, FONT_FILE_NAME);
    SDL_Texture * snake   = create_texture(renderer, TEXTURE_FILE_NAME);

    Image * textures[Texture_COUNT] = { 0 };
    load_game_textures(renderer, snake, textures);

    int exit_code = main_loop(renderer, &game, charmap, textures);

    destroy_game_textures(textures);

    SDL_DestroyTexture(charmap);
    
    SDL_DestroyWindow(window);
    //Quit SDL subsystems
    SDL_Quit();
    if (game.snake != NULL) {
        free(game.snake);
    }

    return exit_code;
}
