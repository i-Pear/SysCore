#include "memory_keeper.h"

void _MemKeeper::add(void* p){
    elements.push_back(p);
}

void _MemKeeper::erase(void* p){
    elements.erase(p);
}

_MemKeeper::~_MemKeeper(){
    int length=elements.size;
    for(int i=0;i<length;i++){
        delete (size_t*)elements[i];
    }
}