#ifndef POINT_H
#define POINT_H

typedef struct {
    int x, y;
} Point;


typedef enum {
    Right = 0,
    Down,
    Left,
    Up,
    Unknown,
} Direction;

#define get_direction(a, b) _get_direction((Point *)a, (Point *)b)
Direction _get_direction(Point * a, Point * b);

#endif