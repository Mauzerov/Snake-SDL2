#ifndef CONFIG_H
#define CONFIG_H


#define GAME_SIZE       16 // playable area size
#define TILE_SIZE       32 // pixels
#define TEXTURE_SIZE    32 // pixels
#define CHAR_WIDTH      8  // pixels
#define INFO_CHAR_SIZE  (TILE_SIZE >> 1)
#define CHARMAP_SIZE    16 // number of characters per column/row in charmap.bmp
#define CHARMAP_MASK    (CHARMAP_SIZE * CHARMAP_SIZE - 1)
#define INFO_PANEL_ROWS 3
#define INFO_PANEL_SIZE INFO_CHAR_SIZE * INFO_PANEL_ROWS
#define GAME_WIDTH      (GAME_SIZE * TILE_SIZE)

#define MAX_STRING_BUFFER_SIZE 1024

#define SAVE_FILE_NAME       "snake_game.save"
#define CONF_FILE_NAME       "snake.config"
#define LEADERBOARD_FILENAME "snake.score"

#define SHORTEN_BY 4
#define INITIAL_SNAKE_SIZE 5
#define INITIAL_SNAKE_X    0
#define INITIAL_SNAKE_Y    0

#define UNDEFINED_POS -100

#define REFRESH_RATE 60
#define LAZY_DELAY 100 // ms
#define FRAMES_PER_SECOND 5
#define FRAMES_MAX_COUNT  100000
#define SCALE_INTERVAL     10 // seconds
#define INITIAL_TIME_SCALE 1.0
#define TIME_SCALE         1.2


#define APPLE_SCORE 2
#define BERRY_SCORE 1

#define APPLE_TIMER_WIDTH 16
#define APPLE_TIMER_CAP   40 // seconds
#define APPLE_SHOW_CHANCE 1 // percent
#define ANIMATION_SIZE    2 // pixels (how much bigger)
#define ANIMATION_INTERVAL .5 // seconds

#define SNAKE_ANIMATION_SIZE    2 // pixels (how many rows of texture file to load)

#define WINDOW_WIDTH    (GAME_WIDTH + APPLE_TIMER_WIDTH)
#define WINDOW_HEIGHT   (GAME_WIDTH + INFO_PANEL_SIZE)

#define PORTER_COUNT 2
#define MAX_NAME_SIZE    20
#define LEADERBOARD_SIZE 3

#define rgb(r, g, b) r, g, b
#define rgba(r, g, b, a) r, g, b, (int)(a * 255)

/// colors from: https://flatuicolors.com/palette/se
#define Color_SNAKE_TAIL (SDL_Color) { rgb(11 , 232, 129), 255 }
#define Color_SNAKE_HEAD (SDL_Color) { rgb(5  , 196, 107), 255 }
#define Color_APPLE      (SDL_Color) { rgb(255, 63 , 52 ), 255 }
#define Color_BERRY      (SDL_Color) { rgb(60 , 64 , 198), 255 }
#define Color_BACKGROUND (SDL_Color) { rgb(30 , 39 , 46 ), 255 }
#define Color_BLACK      (SDL_Color) { rgb(0  , 0  , 0  ), 255 }
#define Color_FOREGROUND (SDL_Color) { rgb(210, 218, 226), 255 }
#define Color_PORTER     (SDL_Color) { rgb(239,  87, 119), 255 }

#define FINISHING 0
#define FINISHED -1
#define TRUE      1

#endif