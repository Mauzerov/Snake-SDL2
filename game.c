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

void reset_game(Game * g) {

}

void new_game(Game * game) {
    game->snake_size = INITIAL_SNAKE_SIZE;
    game->dx = game->dy = 0;
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
        "Optional:  A,B,C,D,E,F,G,H"
    );
    SDL_RenderText(
        renderer, charmap,
        string, fg,
        0, GAME_WIDTH + FONT_SIZE, FONT_SIZE
    );
}