#ifndef OS_RISC_V_STRUCT_INTEGER_LIST_H
#define OS_RISC_V_STRUCT_INTEGER_LIST_H

#include "stdbool.h"
#include "stddef.h"

typedef struct size_t_listNode{
    size_t data;
    struct size_t_listNode* next;
} size_t_listNode;

typedef struct {
    size_t_listNode* start;
    size_t_listNode* end;
} size_t_List;

bool size_t_list_is_empty(size_t_List* list);

void size_t_push_back(size_t_List* list, size_t data);

void size_t_push_front(size_t_List* list, size_t data);

void size_t_list_pop_front(size_t_List* list);

void size_t_list_copy(size_t_List* origin,size_t_List* dest);

#endif //OS_RISC_V_STRUCT_INTEGER_LIST_H
