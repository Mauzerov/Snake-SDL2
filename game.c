#include <time.h>

#include "config.h"
#include "game.h"
#include "snake.h"
#include "image.h"


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
    game->time_scale = INITIAL_TIME_SCALE;
    game->ongoing = TRUE;
    game->seed = time( NULL );
    game->text_entered = false;
    game->apple_timer = game->score = 0;

    memset(game->buffer, 0, sizeof(game->buffer));

    srand(game->seed);
    
    snake_init(&(game->snake), game->snake_size);   
    porters_init(game);

    game->apple.x = game->apple.y = -100;
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
}