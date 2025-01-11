#ifndef ENTITY_H
#define ENTITY_H

typedef struct Porter {
    int x, y; // indirect inheritance (Point)
    int identifier;
    struct Porter * destination;
} Porter;

typedef struct {
    int x, y; // indirect inheritance (Point)
    int animation_frame;
} Entity;

#endif