#include <time.h>

#include "config.h"
#include "game.h"
#include "snake.h"
#include "image.h"
#include "renderer.h"


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
    game->elapsed_time = 0.f;
    game->time_scale = INITIAL_TIME_SCALE;
    game->ongoing = TRUE;
    game->seed = time( NULL );
    game->text_entered = false;
    game->apple_timer = game->score = 0;

    memset(game->buffer, 0, sizeof(game->buffer));

    srand(game->seed);
    
    snake_init(&(game->snake), game->snake_size);   
    porters_init(game);

    game->apple.x = game->apple.y = UNDEFINED_POS;
    random_position(game, &game->berry);
    game->records = read_leaderboard(game->leaderboard);
}

void load_game_textures(
    SDL_Renderer * renderer,
    Game * game,
    SDL_Texture * texture
) {
    for (int i = Texture_TAIL; i <= Texture_HEAD; i++) {
        game->textures[i] = create_image(
            renderer, texture,
            (SDL_Rect) {
                TEXTURE_SIZE * i, 0,
                TEXTURE_SIZE, TEXTURE_SIZE * SNAKE_ANIMATION_SIZE
            },
            Color_SNAKE_TAIL
        );
    }

    SDL_Rect fruit_rect = {
        TEXTURE_SIZE * Texture_APPLE, 0,
        TEXTURE_SIZE, TEXTURE_SIZE * 2
    };

    game->textures[Texture_APPLE] = create_image(
        renderer, texture,
        fruit_rect,
        Color_APPLE
    );
    game->textures[Texture_BERRY] = create_image(
        renderer, texture,
        fruit_rect,
        Color_BERRY
    );

    game->textures[Texture_PORTER] = create_image(
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

void destroy_game_textures(Game * game) {
    for (int i = Texture_TAIL; i < Texture_COUNT; i++)
        destroy_image(game->textures[i]);
}


void render_game(SDL_Renderer * renderer, Game * game, SDL_Texture * charmap) {
    SDL_Color bg = Color_BACKGROUND;

    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderClear(renderer);
    draw_apple_timer(renderer, game);

    for (int i = 0; i < PORTER_COUNT * 2; i++) {
        draw_porter(renderer, charmap, game, &game->porters[i]);
    }

    draw_entity(renderer, &(game->apple), game->textures[Texture_APPLE]);
    draw_entity(renderer, &(game->berry), game->textures[Texture_BERRY]);

    render_snake(renderer, game->snake, game->snake_size, game->textures);

    render_game_info(renderer, game, charmap);
    
    SDL_RenderPresent(renderer);
}


void render_game_info(SDL_Renderer * renderer, Game * game, SDL_Texture * charmap) {
    SDL_Color fg = Color_FOREGROUND;
    SDL_Color bg = Color_BLACK;

    SDL_Rect bg_rect = (SDL_Rect) { 0, GAME_WIDTH, WINDOW_WIDTH, INFO_PANEL_SIZE };
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderFillRect(renderer, &bg_rect);

    static char string[64] = { 0 };

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
        0, GAME_WIDTH, INFO_CHAR_SIZE
    );
    
    SDL_RenderText(
        renderer, charmap,
        string, fg,
        GAME_WIDTH - (sprintf(
            string,
            "Score: %04lu\nSpeed: %.2f",
            game->score, game->time_scale
        ) * INFO_CHAR_SIZE >> 1),
        GAME_WIDTH, INFO_CHAR_SIZE
    );

    sprintf(string,
        "Mandatory: 1,2,3,4\n"
        "Optional:  A,B,C,D,E,F,G,H"
    );
    SDL_RenderText(
        renderer, charmap,
        string, fg,
        0, GAME_WIDTH + INFO_CHAR_SIZE, INFO_CHAR_SIZE
    );
}