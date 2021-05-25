#ifndef OS_RISC_V_STRUCT_INTEGER_MAP_H
#define OS_RISC_V_STRUCT_INTEGER_MAP_H

#include "stddef.h"
#include "stdbool.h"

typedef struct size_t_mapNode{
    size_t key,value;
    struct size_t_mapNode* next;
} size_t_mapNode;

typedef struct {
    size_t_mapNode* start;
} size_t_map;

void size_t_map_copy(size_t_map* source,size_t_map* dest);

void size_t_map_init(size_t_map* map);

void size_t_map_put(size_t_map* map,size_t key,size_t value);

void size_t_map_push_front(size_t_map* map,size_t key,size_t value);

void size_t_map_erase(size_t_map* map,size_t key);

bool size_t_map_exists(size_t_map* map,size_t key);

size_t size_t_map_get(size_t_map* map,size_t key);

#endif //OS_RISC_V_STRUCT_INTEGER_MAP_H
