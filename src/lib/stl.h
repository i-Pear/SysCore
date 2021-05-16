#ifndef __STL_H__
#define __STL_H__

#include "memory.h"

typedef struct {
    char *start;
    size_t length;
    size_t capacity;
    size_t type_size;
} Vec;

void Vec_init(Vec **vec, size_t type_size);

void Vec_push(Vec *vec, void *x);

void Vec_pop(Vec *vec, void *x);

void Vec_get(Vec *vec, size_t index, void *x);

void Vec_set(Vec *vec, size_t index, void *x);

void Vec_clear(Vec *vec);

void Vec_free(Vec **vec);

size_t Vec_empty(Vec *vec);

typedef struct MapNode {
    size_t key;
    size_t value;
    struct MapNode *left;
    struct MapNode *right;
} MapNode;

typedef struct {
    MapNode *start;
    size_t length;
} Map;

void Map_init(Map **map);

void Map_put(Map *map, size_t key, size_t value);

int Map_get(Map *map, size_t key, size_t *value);

#endif