#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include <SDL2/SDL.h>

#define GAME_SIZE       16 // playable area size
#define TILE_SIZE       32 // pixels
#define CHAR_WIDTH      8  // pixels
#define CHAR_HEIGHT     16 // pixels
#define CHARMAP_SIZE    16 // number of characters per column/row in charmap.bmp
#define CHARMAP_MASK    (CHARMAP_SIZE * CHARMAP_SIZE - 1)
#define INFO_PANEL_ROWS 3
#define INFO_PANEL_SIZE CHAR_HEIGHT * INFO_PANEL_ROWS
#define GAME_WIDTH      (GAME_SIZE * TILE_SIZE)

#define SAVE_FILE_NAME "snake_game.save"
#define CONF_FILE_NAME "snake.config"

#define SHORTEN_BY 2
#define INITIAL_SNAKE_SIZE 5
#define INITIAL_SNAKE_X    0
#define INITIAL_SNAKE_Y    0

#define FRAMES_PER_SECOND 5
#define FRAMES_MAX_COUNT  100000

#define APPLE_SCORE 2
#define BERRY_SCORE 1

#define APPLE_TIMER_WIDTH 16
#define APPLE_TIMER_CAP   20 // seconds
#define TIME_SCALE        .2
#define SCALE_INTERVAL    60 // seconds
#define ANIMATION_SIZE    2 // pixels (how much bigger)
#define ANIMATION_LENGHT  ((ANIMATION_SIZE << 1) - 1)

#define WINDOW_WIDTH    (GAME_WIDTH + APPLE_TIMER_WIDTH)
#define WINDOW_HEIGHT   (GAME_WIDTH + INFO_PANEL_SIZE)

#define PORTER_COUNT 2

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
} Point;

typedef struct Porter {
    int x, y; // indirect inheritance (Point)
    int identifier;
    struct Porter * destination;
} Porter;

typedef struct {
    int x, y; // indirect inheritance (Point)
    int animation_frame;
    SDL_Color color;
} Entity;

typedef struct Game {
    Entity apple;
    Entity berry;
    Entity * snake;
    int snake_size;
    int apple_timer;
    int score;
    struct tm elapsed_time;
    float time_scale;
    int seed;
    Porter porters[PORTER_COUNT * 2];
    int dx, dy;
    bool ongoing;
    void (*apple_actions[2])(struct Game*);
} Game;

#define _is_outofbounds(a) \
    ((a)->x < 0 || (a)->x >= GAME_SIZE || (a)->y < 0 || (a)->y >= GAME_SIZE)
#define is_outofbounds(a) _is_outofbounds((Point*)a)

#define _is_overlapping(a, b) \
    ((a)->x == (b)->x && (a)->y == (b)->y)
#define is_overlapping(a, b) _is_overlapping((Point*)a, (Point*)b)

int save_rand(Game * game) {
    return game->seed = rand();
}

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
    int x, int y, int out_width
) {
    float scale = out_width / (float) CHAR_WIDTH;
    SDL_Rect src_rect = { 0, 0, CHAR_WIDTH, CHAR_WIDTH },
             dst_rect = { x, y, CHAR_WIDTH * scale, CHAR_WIDTH * scale };

    SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, 255);
    // TODO: implement proper color scaling
    //       above code doesn't do anything

    for (int c; (c = (int)*(string++) & CHARMAP_MASK) != '\0'; ) {
        if ((char)c == '\n') {
            dst_rect.x = x;
            dst_rect.y += out_width;
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
        GAME_WIDTH - sprintf(string, "Score: %04d", game->score) * CHAR_HEIGHT,
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
    SDL_Color color = game->apple.color;
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

bool random_position(Game * game, Entity * entity) {
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
            if ((Entity *)&game->porters[i] != entity && is_overlapping(&game->snake[i], &game->porters[i])) {
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

#define save_file_operattion(file_fn, file, g, game) do {   \
    file_fn(file, "%d %d\n", g->apple.x, g->apple.y);       \
    file_fn(file, "%d %d\n", g->berry.x, g->berry.y);       \
    file_fn(file, "%d\n", g->snake_size);                   \
    /* note that in the for loop `game` is used */          \
    for (int i = 0; i < game->snake_size; i++) {            \
        file_fn(file, "%d %d\n",                            \
            g->snake[i].x, g->snake[i].y);                  \
    }                                                       \
    file_fn(file, "%d %d %d %f\n",                          \
        g->dx, g->dy,                                       \
        g->apple_timer, g->time_scale);                     \
    file_fn(file, "%d %d\n", g->score, g->seed);            \
    file_fn(file, "%d %d\n",                                \
        g->elapsed_time.tm_min,                             \
        g->elapsed_time.tm_sec);                            \
    for (int i = 0; i < PORTER_COUNT * 2; i++) {            \
        file_fn(file, "%d %d %d\n",                         \
            g->porters[i].x, g->porters[i].y,               \
            g->porters[i].identifier);                      \
    }                                                       \
} while (0)

void save_game(Game * game) {
    FILE * file = fopen(SAVE_FILE_NAME, "w");
    save_file_operattion(fprintf, file, game, game);
    fclose(file);
}

void load_game(Game * game) {
    FILE * file = fopen(SAVE_FILE_NAME, "r");
    if (file == NULL) {
        fprintf(stderr, SAVE_FILE_NAME " doesn't exist");
        return;
    }
    save_file_operattion(fscanf, file, &game, game);
    srand(game->seed);
    fclose(file);
}

void porters_init(Game * game) {
    for (int i = 0; i < PORTER_COUNT * 2; i += 2) {
        game->porters[  i  ].destination = &game->porters[i + 1];
        game->porters[i + 1].destination = &game->porters[  i  ];
        game->porters[i].identifier = game->porters[i + 1].identifier = (i >> 1) + '1';

        random_position(game, (Entity*)&game->porters[  i  ]);
        random_position(game, (Entity*)&game->porters[i + 1]);
    }
}

void new_game(Game * game) {
    game->snake_size = INITIAL_SNAKE_SIZE;
    game->dx = game->dy = 0;
    game->elapsed_time = (struct tm) { 0 };
    game->time_scale = 1.0;
    game->ongoing = true;
    game->seed = time( NULL );
    game->apple_timer = game->score = 0;

    srand(game->seed);
    
    snake_init(&(game->snake), game->snake_size);   
    porters_init(game);

    game->apple.x = game->apple.y = -100;
    random_position(game, &game->berry);
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
            game->ongoing = false;
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
            printf("Press 'n' to start a new game!\n"); 
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
    // srand(time( NULL ));

    SDL_Window * window     = NULL;
    SDL_Renderer * renderer = NULL;
    
    Game game = (Game) {
        .apple = (Entity) { 0, .color = Color_APPLE },
        .berry = (Entity) { 0, .color = Color_BERRY },
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

    SDL_Surface * charmap_surface = SDL_LoadBMP("charmap.bmp");

    if (charmap_surface == NULL) {
        fprintf(stderr, "Could Not Initialize!: %d %s\n", __LINE__, SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetColorKey(
        charmap_surface,
        SDL_TRUE,
        SDL_MapRGB(charmap_surface->format, 0, 0, 0)
    );

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