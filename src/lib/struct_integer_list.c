#include "struct_integer_list.h"
#include "kernel_heap.h"

bool size_t_list_is_empty(size_t_List* list){
    return list->start==null;
}

void size_t_push_back(size_t_List* list, size_t data){
    if(list->start==null&&list->end==null){
        // empty list
        size_t_listNode* new_node=k_malloc(sizeof(size_t_listNode));
        new_node->data=data;
        new_node->next=null;

        list->start=list->end=new_node;
    }else{
        size_t_listNode* new_node=k_malloc(sizeof(size_t_listNode));
        new_node->data=data;
        // link
        list->end->next=new_node;
        list->end=new_node;
    }
}

void size_t_push_front(size_t_List* list, size_t data){
    if(list->start==null&&list->end==null){
        // empty list
        size_t_listNode* new_node=k_malloc(sizeof(size_t_listNode));
        new_node->data=data;
        new_node->next=null;

        list->start=list->end=new_node;
    }else{
        size_t_listNode* new_node=k_malloc(sizeof(size_t_listNode));
        new_node->data=data;
        // link
        new_node->next=list->start;
        list->start=new_node;
    }
}

void size_t_list_pop_front(size_t_List* list){
    if(list->start==list->end){
        // has only one
        size_t_listNode* firstNode=list->start;
        list->start=list->end=null;
        k_free(firstNode);
    }else{
        size_t_listNode* firstNode=list->start;
        list->start=firstNode->next;
        k_free(firstNode);
    }
}

void size_t_list_copy(size_t_List* origin,size_t_List* dest){
    size_t_listNode * cnt=origin->start;
    while (cnt!=null){
        size_t_push_back(dest,cnt->data);
    }
}
