#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include <SDL2/SDL.h>

#define GAME_SIZE       16 // playable area size
#define TILE_SIZE       32 // pixels
#define CHAR_WIDTH      8
#define CHAR_HEIGHT     16
#define CHARMAP_SIZE    16
#define CHARMAP_MASK    (CHARMAP_SIZE * CHARMAP_SIZE - 1)
#define INFO_PANEL_ROWS 3
#define INFO_PANEL_SIZE CHAR_HEIGHT * INFO_PANEL_ROWS
#define WINDOW_WIDTH    (GAME_SIZE * TILE_SIZE)
#define WINDOW_HEIGHT   (WINDOW_WIDTH + INFO_PANEL_SIZE)

#define SAVE_FILE_NAME "snake_game.save"
#define CONF_FILE_NAME "snake.config"

#define INITIAL_SNAKE_SIZE 2
#define INITIAL_SNAKE_X    0
#define INITIAL_SNAKE_Y    0

#define TIME_SCALE        .2
#define FRAMES_PER_SECOND 5
#define ANIMATION_SIZE    2 // pixels (how much bigger)
#define ANIMATION_LENGHT  ((ANIMATION_SIZE << 1) - 1)

const unsigned long target_frame_duration = 1000 / FRAMES_PER_SECOND;

/// colors from: https://flatuicolors.com/palette/se
#define Color_SNAKE_TAIL (SDL_Color) { 11 , 232, 129, 0 }
#define Color_SNAKE_HEAD (SDL_Color) { 5  , 196, 107, 0 }
#define Color_APPLE      (SDL_Color) { 255, 63 , 52 , 0 }
#define Color_BERRY      (SDL_Color) { 60 , 64 , 198, 0 }
#define Color_BACKGROUND (SDL_Color) { 30 , 39 , 46 , 0 }
#define Color_BLACK      (SDL_Color) { 0  , 0  , 0  , 0 }
#define Color_FOREGROUND (SDL_Color) { 210, 218, 226, 0 }

#define inctime(time, seconds) do { \
    time.tm_sec += seconds;         \
    mktime(&time);                  \
} while ( 0 )

typedef struct {
    int x, y;
    int animation_frame;
    SDL_Color color;
} Entity;

typedef struct {
    Entity apple;
    Entity berry;
    Entity * snake;
    int snake_size;
    struct tm elapsed_time;
    float time_scale;
    int dx, dy;
    bool ongoing;
} Game;

#define is_outofbounds(a) \
    ((a)->x < 0 || (a)->x >= GAME_SIZE || (a)->y < 0 || (a)->y >= GAME_SIZE)

#define is_overlapping(a, b) \
    ((a)->x == (b)->x && (a)->y == (b)->y)

void draw_entity(SDL_Renderer * renderer, Entity * entity) {
    SDL_Color color = entity->color;
    SDL_Rect fill_rect, border_rect;

    entity->animation_frame = entity->animation_frame % ANIMATION_LENGHT;
    int frame = labs(entity->animation_frame - ANIMATION_SIZE);

    fill_rect = (SDL_Rect) {
        entity->x * TILE_SIZE - frame,
        entity->y * TILE_SIZE - frame,
        TILE_SIZE + 2 * frame,
        TILE_SIZE + 2 * frame,
    };

    border_rect = (SDL_Rect) {
        fill_rect.x - 1,
        fill_rect.y - 1,
        fill_rect.w + 2,
        fill_rect.h + 2,
    };

    SDL_SetRenderDrawColor(renderer, color.r * .9, color.g * .9, color.b * .9, 255);
    SDL_RenderDrawRect(renderer, &border_rect);

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &fill_rect);
}

int SDL_RenderText(
    SDL_Renderer * renderer,
    SDL_Texture * charmap,
    const char * string,
    SDL_Color fg,
    int x,
    int y
) {
    static float scale = CHAR_HEIGHT / (float) CHAR_WIDTH;
    SDL_Rect src_rect = { 0, 0, CHAR_WIDTH, CHAR_WIDTH },
             dst_rect = { x, WINDOW_WIDTH + y, CHAR_WIDTH * scale, CHAR_WIDTH * scale };

    SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, 255);

    for (int c; (c = (int)*(string++) & CHARMAP_MASK) != '\0'; ) {
        if ((char)c == '\n') {
            dst_rect.x = x;
            dst_rect.y += CHAR_HEIGHT;
            continue;
        }
        src_rect.x = (c % CHARMAP_SIZE) * CHAR_WIDTH;
        src_rect.y = (c / CHARMAP_SIZE) * CHAR_WIDTH;
        SDL_RenderCopy(renderer, charmap, &src_rect, &dst_rect);
        dst_rect.x += CHAR_WIDTH * scale;
    }
    return 0;
}

void draw_text(SDL_Renderer * renderer, Game * game, SDL_Texture * charmap) {
    SDL_Color fg = Color_FOREGROUND;
    SDL_Color bg = Color_BLACK;

    SDL_Rect bg_rect = (SDL_Rect) { 0, WINDOW_WIDTH, WINDOW_WIDTH, INFO_PANEL_SIZE };
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderFillRect(renderer, &bg_rect);

    static char string[64] = { 0 };

    strftime(string, sizeof(string),
        "Elapsed Time: %M:%S", &game->elapsed_time
    );
    // sprintf(string, "Elapsed Time: %ds", game->elapsed_time.tm_sec);
    SDL_RenderText(renderer, charmap, string, fg, 0, 0);

    sprintf(string,
        "Mandatory: 1,2,3,4 %f\n"
        "Optional:  A,B,C,D,E,F,G,H", game->time_scale
    );
    SDL_RenderText(renderer, charmap, string, fg, 0, CHAR_HEIGHT);
}

void render_frame(SDL_Renderer * renderer, Game * game, SDL_Texture * charmap) {
    SDL_Color bg = Color_BACKGROUND;

    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderClear(renderer);

    Entity * snake = game->snake;

    draw_entity(renderer, &(game->apple));
    draw_entity(renderer, &(game->berry));
    
    for (int i = 1; i < game->snake_size; i++) {
        draw_entity(renderer, &(snake[i]));
    }
    draw_entity(renderer, &(snake[0]));

    draw_text(renderer, game, charmap);

    SDL_RenderPresent(renderer);
}

void on_outofbounds(Game * game) {
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

bool random_position(Game * game, Entity * entity) {
    bool valid = false;
    if (game->snake_size > GAME_SIZE * GAME_SIZE)
        return valid;
    do {
        valid = true;
        entity->x = rand() % GAME_SIZE;
        entity->y = rand() % GAME_SIZE;

        for (int i = 0; i < game->snake_size; i++) {
            if (is_overlapping(&(game->snake[i]), entity)) {
                valid = false;
                break;
            }
        }
    } while (!valid);
    return valid;
}

void snake_init(Entity ** snake, int size) {
    // if snake is not initilized
    // then malloc it
    // if snake is already initialized we can ignore it
    //    due to the fact that next realloc call will truncate the size
    if (*snake == NULL)
        *snake = malloc(sizeof(Entity) * size);

    for (int i = 0; i < size; i++) {
        (*snake)[i].x = INITIAL_SNAKE_X + (size - i - 1);
        (*snake)[i].y = INITIAL_SNAKE_Y;
        (*snake)[i].color = Color_SNAKE_TAIL;
    }
    (*snake)[0].color = Color_SNAKE_HEAD;
}

void save_game(Game * game) {
    FILE * file = fopen(SAVE_FILE_NAME, "w");
    (void)game;
    fclose(file);
}

void load_game(Game * game) {
    FILE * file = fopen(SAVE_FILE_NAME, "r");

    (void)game;
    if (file == NULL) {
        fprintf(stderr, SAVE_FILE_NAME " doesn't exist");
        return;
    }
    fclose(file);
}

void new_game(Game * game) {
    game->snake_size = INITIAL_SNAKE_SIZE;
    game->dx = game->dy = 0;
    game->elapsed_time = (struct tm) { 0 };
    game->time_scale = 1.0;
    game->ongoing = true;
    
    snake_init(&(game->snake), game->snake_size);   

    random_position(game, &game->apple);
    random_position(game, &game->berry);
}

void snake_resize(Entity ** snake, int * snake_size) {
    *snake = realloc(*snake, (++(*snake_size)) * sizeof(Entity));
}

void snake_move(Game * game) {
    int dx = game->dx, dy = game->dy;
    assert((dx != 0 || dy != 0) && "snake_move should only be called when the snek can move");

    int * snake_size = &(game->snake_size);
    Entity ** snake = &(game->snake);

    for (int i = 1; i < *snake_size; i++) {
        if (is_overlapping(*snake, &(*snake)[i])) {
            game->ongoing = false;
            return;
        }
    }

    Entity * head = *snake;
    int headx = head->x + dx;
    int heady = head->y + dy;
    head->color = Color_SNAKE_TAIL; // reset head color


    if (is_overlapping(*snake, &(game->apple))) {
        snake_resize(snake, snake_size);
        
        game->ongoing = random_position(game, &(game->apple));
    }
    if (is_overlapping(*snake, &(game->berry))) {
        snake_resize(snake, snake_size);

        game->ongoing = random_position(game, &(game->berry));
    }

    memmove((*snake) + 1, (*snake), sizeof(Entity) * (*snake_size - 1));

    (*snake)[0] = (Entity) { headx, heady, ANIMATION_SIZE, Color_SNAKE_HEAD };

    if (is_outofbounds(&(*snake)[0])) {
        on_outofbounds(game);
    }
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
    return;
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
                handle_keyboard_event(&e.key, game);
            default:
                continue;
            }
        }

        if (!game->ongoing) {
            // printf("Press 'n' to start a new game!\n"); 
            continue;
        }

        if (game->dx != 0 || game->dy != 0) {
            snake_move(game);

            if (elapsed_frames++ % FRAMES_PER_SECOND == 0) {
                int prev_minutes = game->elapsed_time.tm_min;
                inctime(game->elapsed_time, 1);
                if (prev_minutes != game->elapsed_time.tm_min) {
                    game->time_scale += TIME_SCALE;
                }

                elapsed_frames = elapsed_frames % FRAMES_PER_SECOND;
            }
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
        .apple = (Entity) { 0, .color = Color_APPLE },
        .berry = (Entity) { 0, .color = Color_BERRY },
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

    SDL_Surface * charmap_surface = SDL_LoadBMP("charmap.bmp");
    if (charmap_surface == NULL) {
        fprintf(stderr, "Could Not Initialize!: %d %s\n", __LINE__, SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture * charmap = SDL_CreateTextureFromSurface(renderer, charmap_surface);
    if (charmap == NULL) {
        fprintf(stderr, "Could Not Initialize!: %d %s\n", __LINE__, SDL_GetError());
        return EXIT_FAILURE;
    }

    int exit_code = main_loop(renderer, &game, charmap);

    SDL_DestroyTexture(charmap);
    SDL_FreeSurface(charmap_surface);
    SDL_DestroyWindow(window);
    //Quit SDL subsystems
    SDL_Quit();
    if (game.snake != NULL) {
        free(game.snake);
    }

    return exit_code;
}