#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include "engine.h"
#include "config.h"
#include "image.h"
#include "snake.h"
#include "game.h"

#include <SDL2/SDL.h>

const unsigned long target_frame_duration = 1000 / FRAMES_PER_SECOND;

#define inctime(time, seconds) do { \
    time.tm_sec += seconds;         \
    mktime(&time);                  \
} while ( 0 )

void draw_entity(SDL_Renderer * renderer, Entity * entity, Image * texture) {
    entity->animation_frame = entity->animation_frame % ANIMATION_LENGHT;

    SDL_Rect fill_rect = (SDL_Rect) {
        entity->x * TILE_SIZE,
        entity->y * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE,
    };

    render_square_image(
        renderer,
        texture,
        &fill_rect,
        entity->animation_frame,
        0
    );
}

void render_game_info(SDL_Renderer * renderer, Game * game, SDL_Texture * charmap) {
    SDL_Color fg = Color_FOREGROUND;
    SDL_Color bg = Color_BLACK;

    SDL_Rect bg_rect = (SDL_Rect) { 0, GAME_WIDTH, WINDOW_WIDTH, INFO_PANEL_SIZE };
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderFillRect(renderer, &bg_rect);

    static char string[64] = { 0 };

    strftime(string, sizeof(string),
        "Elapsed Time: %M:%S", &game->elapsed_time
    );
    SDL_RenderText(
        renderer, charmap,
        string, fg,
        0, GAME_WIDTH, CHAR_HEIGHT
    );

    SDL_RenderText(
        renderer, charmap,
        string, fg,
        GAME_WIDTH - sprintf(string, "Score: %04lu", game->score) * CHAR_HEIGHT,
        GAME_WIDTH, CHAR_HEIGHT
    );

    sprintf(string,
        "Mandatory: 1,2,3,4\n"
        "Optional:  A,B,C,D,E,F,G,H"
    );
    SDL_RenderText(
        renderer, charmap,
        string, fg,
        0, GAME_WIDTH + CHAR_HEIGHT, CHAR_HEIGHT
    );
}

void draw_apple_timer(SDL_Renderer * renderer, Game * game) {
    SDL_Color color = Color_APPLE;
    SDL_Color bg = Color_BLACK;

    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);

    SDL_Rect rect = (SDL_Rect) {
        .x = GAME_WIDTH,
        .y = 0,
        .w = APPLE_TIMER_WIDTH,
        .h = GAME_WIDTH,
    };
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

    rect.h = GAME_WIDTH / (
        (APPLE_TIMER_CAP * FRAMES_PER_SECOND) / (float)game->apple_timer
    );

    if (game->dx || game->dy)
        game->apple_timer++;

    SDL_RenderFillRect(renderer, &rect);
}

void draw_porter(SDL_Renderer * renderer, SDL_Texture * charmap, Porter * porter) {
    SDL_Color fg = Color_FOREGROUND;
    char string[2] = { (char)porter->identifier, 0 };
    SDL_RenderText(
        renderer, charmap, 
        string, fg,
        porter->x * TILE_SIZE, porter->y * TILE_SIZE, TILE_SIZE
    );
}

void render_frame(SDL_Renderer * renderer, Game * game, SDL_Texture * charmap) {
    SDL_Color bg = Color_BACKGROUND;

    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderClear(renderer);
    draw_apple_timer(renderer, game);

    for (int i = 0; i < PORTER_COUNT * 2; i++) {
        draw_porter(renderer, charmap, &game->porters[i]);
    }

    draw_entity(renderer, &(game->apple), game->textures[Texture_APPLE]);
    draw_entity(renderer, &(game->berry), game->textures[Texture_BERRY]);

    render_snake(renderer, game->snake, game->snake_size, game->textures);

    render_game_info(renderer, game, charmap);
    
    SDL_RenderPresent(renderer);
}

void render_leaderboard(
    SDL_Renderer * renderer,
    SDL_Texture * charmap,
    Game * game,
    int records,
    int top_position
) {
    char buffer[MAX_NAME_SIZE * 2] = { 0 };
    for (int i = 0; i < records; i++) {
        sprintf(
            buffer, "   %d. %-*s: %lu", i + 1,
            MAX_NAME_SIZE,
            game->leaderboard[i].name,
            game->leaderboard[i].score
        );
        SDL_RenderText(
            renderer, charmap, buffer, Color_FOREGROUND,
            0, top_position + (8 + i) * CHAR_HEIGHT, CHAR_HEIGHT
        );
    }
}

void render_end_screen(SDL_Renderer * renderer, SDL_Texture * charmap, Game * game) {
    int line_num = 8 + LEADERBOARD_SIZE;

    SDL_Color bg = Color_BLACK;
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);

    int top_position = (GAME_WIDTH >> 1) - (line_num) * CHAR_HEIGHT;
    SDL_Rect rect = { 
        0, top_position, 
        GAME_WIDTH, CHAR_HEIGHT * (2 + line_num)
    };
    SDL_RenderFillRect(renderer, &rect);

    char string[255] = { 0 };

    sprintf(
        string,
        " Press 'n' to start a new game!\n"
        "     Press 'ESC' to quit!    \n\n"
        "        Your Score %04lu       \n"
        "         Leaderboard:  \n\n\n\n\n"
        " %s %s ",
        game->score,
        game->text_entered ? "" : "Your Name:",
        game->text_entered ? "" : game->buffer
    );

    SDL_RenderText(
        renderer, charmap, string, Color_FOREGROUND,
        0, top_position + 2 * CHAR_HEIGHT, CHAR_HEIGHT
    );

    render_leaderboard(renderer, charmap, game, game->records, top_position);
    SDL_RenderPresent(renderer);
}

void snake_resize(Entity ** snake, int snake_size) {
    *snake = realloc(*snake, ((snake_size)) * sizeof(Entity));
}

void snake_move(Game * game) {
    int dx = game->dx, dy = game->dy;
    assert((dx != 0 || dy != 0) && "snake_move should only be called when the snek can move");

    int * snake_size = &(game->snake_size);
    Entity ** snake = &(game->snake);

    for (int i = 1; i < *snake_size; i++) {
        if (is_overlapping(*snake, &(*snake)[i])) {
            game->ongoing = 0;
            return;
        }
    }

    Entity * head = *snake;
    int headx = head->x + dx;
    int heady = head->y + dy;
    head->color = Color_SNAKE_TAIL; // reset head color


    if (is_overlapping(*snake, &(game->apple))) {
        int apple_action_index = save_rand(game) % 2;
        game->score += APPLE_SCORE;
        game->apple_actions[apple_action_index](game);
        game->apple.x = game->apple.y = -100;
        game->apple_timer = 0;
    } else if (game->apple_timer == APPLE_TIMER_CAP * FRAMES_PER_SECOND) {
        game->ongoing = random_position(game, &(game->apple));
    }
    if (is_overlapping(*snake, &(game->berry))) {
        snake_resize(snake, *snake_size += 1);
        game->score += BERRY_SCORE;
        game->ongoing = random_position(game, &(game->berry));
    }

    memmove((*snake) + 1, (*snake), sizeof(Entity) * (*snake_size - 1));

    (*snake)[0] = (Entity) { headx, heady, ANIMATION_SIZE, Color_SNAKE_HEAD };

    for (int i = 0; i < PORTER_COUNT * 2; i++) {
        if (is_overlapping(*snake, &game->porters[i])) {
            Porter * porter = &game->porters[i];
            (*snake)[0].x = porter->destination->x;
            (*snake)[0].y = porter->destination->y;
            break;
        }
    }

    if (is_outofbounds(&(*snake)[0])) {
        handle_outofbounds(game);
    }
}

void apple_action_shorten(Game * game) {
    game->snake_size -= SHORTEN_BY;
    if (game->snake_size < INITIAL_SNAKE_SIZE)
        game->snake_size = INITIAL_SNAKE_SIZE;
    snake_resize(&game->snake, game->snake_size);
}

void apple_action_slowdown(Game * game) {
    game->time_scale -= TIME_SCALE / 2.;
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
        exit(0);
        break;
    }
    default:
        return;
    }
}

void handle_text_input(SDL_KeyboardEvent * e, Game * game) {
    int key = e->keysym.sym;
    if (key == SDLK_RETURN) {
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

int main_loop(SDL_Renderer * renderer, Game * game, SDL_Texture * charmap) {
    static SDL_Event e;

    unsigned long frame_start, frame_duration;
    unsigned elapsed_frames = 0;

    while (1) {
        frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return 0;
            }

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

        if (game->ongoing <= FINISHING) {
            if (game->ongoing == FINISHING) {
                game->ongoing = FINISHED;
                game->text_entered = !can_add_to_leaderboard(game);
            }
            render_end_screen(renderer, charmap, game);
            SDL_Delay(100);
            continue;
        }

        if (game->dx != 0 || game->dy != 0) {
            snake_move(game);

            // handle current play time
            if (elapsed_frames++ % FRAMES_PER_SECOND == 0) {
                inctime(game->elapsed_time, 1);
            }
            // handle speed-up
            if (elapsed_frames % (FRAMES_PER_SECOND * SCALE_INTERVAL) == 0) {
                game->time_scale += TIME_SCALE;
            }

            elapsed_frames = elapsed_frames % FRAMES_MAX_COUNT;
        }

        update_animations(game);
        render_frame(renderer, game, charmap);

        frame_duration = SDL_GetTicks() - frame_start;
        if (frame_duration * game->time_scale < target_frame_duration) {
            SDL_Delay(target_frame_duration - frame_duration * game->time_scale);
        }
    }
    return 0;
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

    SDL_DestroyTexture(charmap);
    
    SDL_DestroyWindow(window);
    //Quit SDL subsystems
    SDL_Quit();
    if (game.snake != NULL) {
        free(game.snake);
    }

    return exit_code;
}
