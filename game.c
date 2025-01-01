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
    game->time_scale = 1.0;
    game->ongoing = 1;
    game->seed = time( NULL );
    game->apple_timer = game->score = 0;

    srand(game->seed);
    
    snake_init(&(game->snake), game->snake_size);   
    porters_init(game);

    game->apple.x = game->apple.y = -100;
    random_position(game, &game->berry);
}

void load_game_textures(
    SDL_Renderer * renderer,
    Game * game,
    SDL_Texture * texture
) {
    Image * tail = create_image(
        renderer, texture,
        (SDL_Rect) { 0, 0, TILE_SIZE, TILE_SIZE * 2 },
        Color_SNAKE_TAIL
    );
    Image * body = create_image(
        renderer, texture,
        (SDL_Rect) { TILE_SIZE * 1, 0, TILE_SIZE, TILE_SIZE * 2 },
        Color_SNAKE_TAIL
    );
    Image * head = create_image(
        renderer, texture,
        (SDL_Rect) { TILE_SIZE * 2, 0, TILE_SIZE, TILE_SIZE * 2 },
        Color_SNAKE_TAIL
    );
    Image * apple = create_image(
        renderer, texture,
        (SDL_Rect) { TILE_SIZE * 3, 0, TILE_SIZE, TILE_SIZE * 2 },
        Color_APPLE
    );
    Image * berry = create_image(
        renderer, texture,
        (SDL_Rect) { TILE_SIZE * 3, 0, TILE_SIZE, TILE_SIZE * 2 },
        Color_BERRY
    );

    game->textures[0] = tail;
    game->textures[1] = body;
    game->textures[2] = head;
    game->textures[3] = apple;
    game->textures[4] = berry;
}