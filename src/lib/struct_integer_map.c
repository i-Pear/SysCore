#include "struct_integer_map.h"
#include "kernel_heap.h"

void size_t_map_init(size_t_map* map){
    map->start=null;
}

void size_t_map_copy(size_t_map* source,size_t_map* dest){
    if(source->start==null)return;
    size_t_mapNode* cnt=source->start;
    while (cnt!=null){
        size_t_map_push_front(dest,cnt->key,cnt->value);
        cnt=cnt->next;
    }
}

void size_t_map_put(size_t_map* map,size_t key,size_t value){
    size_t_mapNode* cnt=map->start;
    while (cnt!=null){
        if(cnt->key==key){
            cnt->value=value;
            return;
        }
        cnt=cnt->next;
    }
    // push front
    size_t_mapNode* newNode= k_malloc(sizeof(size_t_mapNode));
    newNode->key=key;
    newNode->value=value;
    newNode->next=map->start;
    map->start=newNode;
}

void size_t_map_push_front(size_t_map* map,size_t key,size_t value){
    size_t_mapNode* newNode= k_malloc(sizeof(size_t_mapNode));
    newNode->key=key;
    newNode->value=value;
    newNode->next=map->start;
    map->start=newNode;
}

void size_t_map_erase(size_t_map* map,size_t key){
    if(map->start==null)return;
    if(map->start->key==key){
        size_t_mapNode* to_erase=map->start;
        map->start=to_erase->next;
        k_free(to_erase);
        return;
    }
    size_t_mapNode* previous=map->start;
    size_t_mapNode* cnt=previous->next;
    while (cnt!=null){
        if(cnt->key==key){
            previous->next=cnt->next;
            k_free(cnt);
            return;
        }
        previous=previous->next;
        cnt=cnt->next;
    }
}

bool size_t_map_exists(size_t_map* map,size_t key){
    size_t_mapNode* cnt=map->start;
    while (cnt!=null){
        if(cnt->key==key){
            return true;
        }
        cnt=cnt->next;
    }
    return false;
}

size_t size_t_map_get(size_t_map* map,size_t key){
    size_t_mapNode* cnt=map->start;
    while (cnt!=null){
        if(cnt->key==key){
            return cnt->value;
        }
        cnt=cnt->next;
    }
    printf("in map_get: key %d not exist!\n", key);
    panic("");
    return 0;
}
