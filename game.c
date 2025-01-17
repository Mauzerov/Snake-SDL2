#include <time.h>

#include "config.h"
#include "game.h"
#include "snake.h"
#include "image.h"
#include "text.h"

/**
 SSSSS   AA   V    V EEEEE     EEEEE IIII LL    EEEEE 
SS      AAAA  VV  VV EE        EE     II  LL    EE    
 SSSS   A  A  VV  VV EEEE      EEEE   II  LL    EEEE  
    SS AAAAAA  VVVV  EE        EE     II  LL    EE    
SSSSS  AA  AA   VV   EEEEE     EE    IIII LLLLL EEEEE
**/

#define save_file_operattion(file_fn, file, g, game) do {   \
    file_fn(file, "%d %d\n", g->apple.x, g->apple.y);       \
    file_fn(file, "%d %d\n", g->berry.x, g->berry.y);       \
    file_fn(file, "%d\n", g->snake_size);                   \
    snake_resize(&game->snake, game->snake_size);           \
    /* note that in the for loop `game` is used */          \
    for (size_t i = 0; i < game->snake_size; i++) {         \
        file_fn(file, "%d %d\n",                            \
            g->snake[i].x, g->snake[i].y);                  \
    }                                                       \
    file_fn(file, "%f\n", g->elapsed_time);                 \
    file_fn(file, "%d %d\n", g->delta.x, g->delta.y);       \
    file_fn(file, "%d %d\n", g->prev.x,  g->prev.y );       \
    file_fn(file, "%f %f %f %f %f\n",                       \
        g->apple_cooldown,                                  \
        g->move_cooldown,                                   \
        g->animation_cooldown,                              \
        g->speedup_cooldown,                                \
        g->game_speed_scale                                 \
    );                                                      \
    file_fn(file, "%lu %d\n", g->score, g->seed);           \
    for (size_t i = 0; i < PORTER_COUNT * 2; i++) {         \
        file_fn(file, "%d %d %d\n",                         \
            g->porters[i].x, g->porters[i].y,               \
            g->porters[i].identifier);                      \
    }                                                       \
    /* ongoing / buffer / leaderboard / textures unchanged*/\
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

/**
IIII NN   N IIII TTTTTT 
 II  NNN  N  II  TTTTTT 
 II  N NN N  II    TT   
 II  N  NNN  II    TT   
IIII N   NN IIII   TT  
**/

void porters_init(Game * game) {
    for (int i = 0; i < PORTER_COUNT * 2; i += 2) {
        game->porters[  i  ].destination = &game->porters[i + 1];
        game->porters[i + 1].destination = &game->porters[  i  ];
        game->porters[i].identifier = game->porters[i + 1].identifier = (i >> 1) + '1';

        random_position(game, (Entity*)&game->porters[  i  ]);
        random_position(game, (Entity*)&game->porters[i + 1]);
    }
}

void apple_action_shorten(Game * game) {
    game->snake_size -= APPLE_SHORTEN_BY;
    if (game->snake_size < INITIAL_SNAKE_SIZE)
        game->snake_size = INITIAL_SNAKE_SIZE;
}

void apple_action_slowdown(Game * game) {
    game->game_speed_scale -= APPLE_SLOWDOWN_BY;
    if (game->game_speed_scale < INITIAL_TIME_SCALE)
        game->game_speed_scale = INITIAL_TIME_SCALE;
}

void new_game(Game * game) {
    game->snake_size = INITIAL_SNAKE_SIZE;
    game->delta.x = game->delta.y = 0;
    game->prev.x = game->prev.y = 0;
    game->elapsed_time = 0.f;
    game->game_speed_scale = INITIAL_TIME_SCALE;
    game->ongoing = TRUE;
    game->seed = time( NULL );
    game->text_entered = false;
    game->apple_cooldown = game->score = 0;
    game->animation_cooldown = 0;
    game->speedup_cooldown = SPEED_SCALE_INTERVAL;
    game->move_cooldown = 1.f / FRAMES_PER_SECOND;

    memset(game->buffer, 0, sizeof(game->buffer));

    srand(game->seed);
    
    snake_init(&(game->snake), game->snake_size);   
    porters_init(game);

    game->apple.x = game->apple.y = UNDEFINED_POS;
    random_position(game, &game->berry);

    game->records = read_leaderboard(game->leaderboard);
}

/**
TTTTTT EEEEE X   XX TTTTTT UU  UU RRRRR  EEEEE  SSSSS 
TTTTTT EE     X XX  TTTTTT UU  UU RR  RR EE    SS     
  TT   EEEE    XX     TT   UU  UU RRRRR  EEEE   SSSS  
  TT   EE     XX X    TT   UUUUUU RR  RR EE        SS 
  TT   EEEEE XX   X   TT    UUUU  RR  RR EEEEE SSSSS  
**/

void load_game_textures(
    SDL_Renderer * renderer,
    SDL_Texture * texture,
    Image * textures[Texture_COUNT]
) {
    for (int i = Texture_TAIL; i <= Texture_HEAD; i++) {
        textures[i] = create_image(
            renderer, texture,
            (SDL_Rect) {
                TEXTURE_SIZE * i, 0,
                TEXTURE_SIZE, -1
            },
            Color_SNAKE_TAIL
        );
    }

    SDL_Rect fruit_rect = {
        TEXTURE_SIZE * Texture_APPLE, 0,
        TEXTURE_SIZE, -1
    };

    textures[Texture_APPLE] = create_image(
        renderer, texture,
        fruit_rect,
        Color_APPLE
    );
    textures[Texture_BERRY] = create_image(
        renderer, texture,
        fruit_rect,
        Color_BERRY
    );

    textures[Texture_PORTER] = create_image(
        renderer, texture,
        (SDL_Rect) {
            TEXTURE_SIZE * Texture_HEAD,
            TEXTURE_SIZE,
            TEXTURE_SIZE,
            TEXTURE_SIZE
        },
        Color_PORTER
    );
}

void destroy_game_textures(Image * textures[Texture_COUNT]) {
    for (int i = 0; i < Texture_COUNT; i++)
        destroy_image(textures[i]);
}

/**
RRRRR  EEEEE NN   N DDDDD  EEEEE RRRRR  
RR  RR EE    NNN  N DD  DD EE    RR  RR 
RRRRR  EEEE  N NN N DD  DD EEEE  RRRRR  
RR  RR EE    N  NNN DD  DD EE    RR  RR 
RR  RR EEEEE N   NN DDDDD  EEEEE RR  RR 
**/

void render_game(
    SDL_Renderer * renderer,
    Game * game,
    SDL_Texture * charmap,
    Image * textures[Texture_COUNT]
) {
    SDL_Color bg = Color_BACKGROUND;

    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderClear(renderer);
    draw_apple_timer(renderer, game);

    for (int i = 0; i < PORTER_COUNT * 2; i++) {
        draw_porter(renderer, charmap, &game->porters[i], textures);
    }

    draw_entity(renderer, &(game->apple), textures[Texture_APPLE]);
    draw_entity(renderer, &(game->berry), textures[Texture_BERRY]);

    render_snake(renderer, game->snake, game->snake_size, textures);

    render_game_info(renderer, game, charmap);
    
    SDL_RenderPresent(renderer);
}


void render_game_info(SDL_Renderer * renderer, Game * game, SDL_Texture * charmap) {
    SDL_Color fg = Color_FOREGROUND;
    SDL_Color bg = Color_BLACK;

    SDL_Rect bg_rect = (SDL_Rect) { 0, GAME_WIDTH, WINDOW_WIDTH, INFO_PANEL_SIZE };
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderFillRect(renderer, &bg_rect);

    static char string[MAX_STRING_BUFFER_SIZE] = { 0 };

    struct tm elapsed_time = (struct tm) {
        .tm_sec = game->elapsed_time
    };
    mktime(&elapsed_time);

    strftime(string, sizeof(string),
        "Elapsed Time: %M:%S", &elapsed_time
    );
    SDL_RenderText(
        renderer, charmap,
        string, fg,
        0, GAME_WIDTH, FONT_SIZE
    );
    
    SDL_RenderText(
        renderer, charmap,
        string, fg,
        GAME_WIDTH - (sprintf(
            string,
            "Score: %04lu\nSpeed: %.2f",
            game->score, game->game_speed_scale
        ) * FONT_SIZE >> 1),
        GAME_WIDTH, FONT_SIZE
    );

    sprintf(string,
        "Mandatory: 1,2,3,4\n"
        "Optional:  A,B,C,D,E,F,G,H\n"
        "Author: Mateusz Mazurek  203223"
    );
    SDL_RenderText(
        renderer, charmap,
        string, fg,
        0, GAME_WIDTH + FONT_SIZE, FONT_SIZE
    );
}