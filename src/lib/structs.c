#include "structs.h"
#include "memory.h"

FileNameList *FileNameListCreate(char *name) {
    FileNameList *res = k_malloc(sizeof(struct FileNameList));
    res->next = NULL;
    strcpy(res->name, name);
    return res;
}


void FileNameListDelete(FileNameList *list) {
    if (list == NULL)return;
    FileNameListDelete(list->next);
    k_free((size_t)list);
}
