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

void apple_action_shorten(Game * game) {
    game->snake_size -= SHORTEN_BY;
    if (game->snake_size < INITIAL_SNAKE_SIZE)
        game->snake_size = INITIAL_SNAKE_SIZE;
    snake_resize(&game->snake, game->snake_size);
}

void apple_action_slowdown(Game * game) {
    game->time_scale -= TIME_SCALE / 2.;
    if (game->time_scale < INITIAL_TIME_SCALE)
        game->time_scale = INITIAL_TIME_SCALE;
}

void update_animations(Game * game) {
    static int animation_frame = 0;
    game->apple.animation_frame++;
    game->berry.animation_frame++;
    // keep the animation frame of the snake (due to movement implementation)
    game->snake[0].animation_frame = animation_frame++;
}

void handle_keyboard_event(SDL_KeyboardEvent * e, Game * game) {
    int * dx = &game->dx, *dy = &game->dy;
    if (e->repeat)
        return;
    switch (e->keysym.sym) {
    // Movement
    case SDLK_a:    
    case SDLK_LEFT:  if (*dx != +1) {
        *dx = -1; *dy = +0;
    }
        break;
    case SDLK_w:
    case SDLK_UP:    if (*dy != +1) {
        *dx = +0;  *dy = -1;
    }
        break;
    case SDLK_d:
    case SDLK_RIGHT: if (*dx != -1) {
        *dx = +1;  *dy = +0;
    }
        break;
    case SDLK_s:
    case SDLK_DOWN:  if (*dy != -1) {
        *dx = +0;  *dy = +1;
    }
        break;
    // Utility 
    case SDLK_n: {
        new_game(game);
        break;
    }
    case SDLK_o: { // swap to 's' before deadline
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

void handle_updates(Game * game, float dt) {
    if (!game->dx && !game->dy)
        return;
    // TODO: move every timed updqde here
    //     : use the -= dt < 0 approach
    //     : reset timers here too
    //     : snake_move should also be called here
    //     : experiment with += for reset instead of just assignent (should prevent desync from real time)
    game->apple_timer -= dt;
    game->elapsed_time += dt;
}

int main_loop(
    SDL_Renderer * renderer,
    Game * game,
    SDL_Texture * charmap
) {
    // TODO: use actual time difference and not a const one
    const float delta_time = 1.f / (float)REFRESH_RATE;
    unsigned elapsed_frames = 0;
    float cooldown = 0.f;

    while (1) {
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

        bool game_running = (game->dx != 0 || game->dy != 0);
        bool should_update = cooldown < 0.f;

        if (game_running && should_update) {
            snake_move(game);
            cooldown = 1.f / (FRAMES_PER_SECOND * game->time_scale);
        } else if (game_running) {
            cooldown -= delta_time;
        }

        if (elapsed_frames % (int)(REFRESH_RATE * ANIMATION_INTERVAL) == 0) {
            update_animations(game);
        }

        if (elapsed_frames % (int)(REFRESH_RATE * SCALE_INTERVAL) == 0) {
            game->time_scale += TIME_SCALE;
        }

        render_game(renderer, game, charmap);
        handle_updates(game, delta_time);
        
        elapsed_frames = (elapsed_frames + 1) % FRAMES_MAX_COUNT;
        SDL_Delay((int)(1000 * delta_time));
    }
}

int main() {
    srand(time( NULL ));

    SDL_Window * window     = NULL;
    SDL_Renderer * renderer = NULL;
    
    Game game = (Game) {
        .apple = (Entity) { 0 },
        .berry = (Entity) { 0 },
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

    SDL_Texture * charmap = create_texture(renderer, "charmap.bmp");

    SDL_Texture * snake   = create_texture(renderer, "snake.bmp");

    load_game_textures(renderer, &game, snake);

    int exit_code = main_loop(renderer, &game, charmap);

    destroy_game_textures(&game);

    SDL_DestroyTexture(charmap);
    
    SDL_DestroyWindow(window);
    //Quit SDL subsystems
    SDL_Quit();
    if (game.snake != NULL) {
        free(game.snake);
    }

    return exit_code;
}
