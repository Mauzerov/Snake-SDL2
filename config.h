#ifndef CONFIG_H
#define CONFIG_H


#define GAME_SIZE       16 // playable area size
#define TILE_SIZE       32 // pixels
#define TEXTURE_SIZE    32 // pixels
#define CHAR_WIDTH      8  // pixels
#define CHAR_HEIGHT     16 // pixels
#define CHARMAP_SIZE    16 // number of characters per column/row in charmap.bmp
#define CHARMAP_MASK    (CHARMAP_SIZE * CHARMAP_SIZE - 1)
#define INFO_PANEL_ROWS 3
#define INFO_PANEL_SIZE CHAR_HEIGHT * INFO_PANEL_ROWS
#define GAME_WIDTH      (GAME_SIZE * TILE_SIZE)

#define SAVE_FILE_NAME       "snake_game.save"
#define CONF_FILE_NAME       "snake.config"
#define LEADERBOARD_FILENAME "snake.score"

#define SHORTEN_BY 2
#define INITIAL_SNAKE_SIZE 5
#define INITIAL_SNAKE_X    0
#define INITIAL_SNAKE_Y    0

#define FRAMES_PER_SECOND 5
#define FRAMES_MAX_COUNT  100000
#define INITIAL_TIME_SCALE 1.0


#define APPLE_SCORE 2
#define BERRY_SCORE 1

#define APPLE_TIMER_WIDTH 16
#define APPLE_TIMER_CAP   20 // seconds
#define TIME_SCALE        .2
#define SCALE_INTERVAL    60 // seconds
#define ANIMATION_SIZE    2 // pixels (how much bigger)
#define ANIMATION_LENGHT  ((ANIMATION_SIZE << 1) - 1)

#define SNAKE_ANIMATION_SIZE    2 // pixels (how many rows of texture file to load)

#define WINDOW_WIDTH    (GAME_WIDTH + APPLE_TIMER_WIDTH)
#define WINDOW_HEIGHT   (GAME_WIDTH + INFO_PANEL_SIZE)

#define PORTER_COUNT 2
#define MAX_NAME_SIZE    20
#define LEADERBOARD_SIZE 3

/// colors from: https://flatuicolors.com/palette/se
#define Color_SNAKE_TAIL (SDL_Color) { 11 , 232, 129, 0 }
#define Color_SNAKE_HEAD (SDL_Color) { 5  , 196, 107, 0 }
#define Color_APPLE      (SDL_Color) { 255, 63 , 52 , 0 }
#define Color_BERRY      (SDL_Color) { 60 , 64 , 198, 0 }
#define Color_BACKGROUND (SDL_Color) { 30 , 39 , 46 , 0 }
#define Color_BLACK      (SDL_Color) { 0  , 0  , 0  , 0 }
#define Color_FOREGROUND (SDL_Color) { 210, 218, 226, 0 }

#define FINISHING 0
#define FINISHED -1
#define TRUE      1

#endif