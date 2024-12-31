#ifndef POINT_H
#define POINT_H

typedef struct {
    int x, y;
} Point;


typedef enum {
    Unknown = 0,
    Right   = 1,
    Down,
    Left,
    Up,
} Direction;

#endif