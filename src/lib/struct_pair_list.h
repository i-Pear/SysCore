#ifndef OS_RISC_V_STRUCT_PAIR_LIST_H
#define OS_RISC_V_STRUCT_PAIR_LIST_H

#include "stddef.h"
#include "stl.h"

typedef struct pair_int_listNode{
    pair_int data;
    struct pair_int_listNode* next;
} pair_int_listNode;

typedef struct {
    pair_int_listNode* start;
    pair_int_listNode* end;
} pair_int_List;

bool pair_int_list_is_empty(pair_int_List* list);

void pair_int_list_push_back(pair_int_List* list, pair_int data);

void pair_int_list_push_front(pair_int_List* list, pair_int data);

void pair_int_list_pop_front(pair_int_List* list);

void pair_int_list_copy(pair_int_List* origin,pair_int_List* dest);

#endif //OS_RISC_V_STRUCT_PAIR_LIST_H
