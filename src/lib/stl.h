#ifndef __STL_H__
#define __STL_H__

#include "memory.h"

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX-1)

typedef struct {
    char *start;
    size_t length;
    size_t capacity;
    size_t type_size;
} Vec;

typedef struct {
    int first,second;
} pair_int;

pair_int make_pair(int a,int b);

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

void memcpy(void *to, void *from, size_t size);

void memset(char* p,char content,int size);

int max(int a,int b);

int min(int a,int b);

#endif