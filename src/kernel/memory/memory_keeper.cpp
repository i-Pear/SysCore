#include "memory_keeper.h"

void MemKeeper::add(void* p){
    elements.push_back(p);
}

void MemKeeper::erase(void* p){
    elements.erase(p);
}

int MemKeeper::size() const{
    return elements.length();
}

MemKeeper::~MemKeeper(){
    int length=elements.size;
    for(int i=0;i<length;i++){
        delete (size_t*)elements[i];
    }
}