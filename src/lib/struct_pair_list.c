#include "struct_pair_list.h"

bool pair_int_list_is_empty(pair_int_List* list){
    return list->start==null;
}

void pair_int_list_push_back(pair_int_List* list, pair_int data){
    if(list->start==null&&list->end==null){
        // empty list
        pair_int_listNode* new_node=k_malloc(sizeof(pair_int_listNode));
        new_node->data=data;
        new_node->next=null;

        list->start=list->end=new_node;
    }else{
        pair_int_listNode* new_node=k_malloc(sizeof(pair_int_listNode));
        new_node->data=data;
        new_node->next=null;
        // link
        list->end->next=new_node;
        list->end=new_node;
    }
}

void pair_int_list_push_front(pair_int_List* list, pair_int data){
    if(list->start==null&&list->end==null){
        // empty list
        pair_int_listNode* new_node=k_malloc(sizeof(pair_int_listNode));
        new_node->data=data;
        new_node->next=null;

        list->start=list->end=new_node;
    }else{
        pair_int_listNode* new_node=k_malloc(sizeof(pair_int_listNode));
        new_node->data=data;
        new_node->next=null;
        // link
        new_node->next=list->start;
        list->start=new_node;
    }
}

void pair_int_list_pop_front(pair_int_List* list){
    if(list->start==list->end){
        // has only one
        pair_int_listNode* firstNode=list->start;
        list->start=list->end=null;
        k_free(firstNode);
    }else{
        pair_int_listNode* firstNode=list->start;
        list->start=firstNode->next;
        k_free(firstNode);
    }
}

void pair_int_list_copy(pair_int_List* origin,pair_int_List* dest){
    pair_int_listNode * cnt=origin->start;
    while (cnt!=null){
        pair_int_list_push_back(dest,cnt->data);
    }
}