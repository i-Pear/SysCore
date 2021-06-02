#include "structs.h"
#include "memory.h"

FileNameList* FileNameListCreate(char* name){
    FileNameList* res = k_malloc(sizeof(struct FileNameList));
    res->next = NULL;
    strcpy(res->name, name);
    return res;
}
