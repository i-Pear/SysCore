#ifndef OS_RISC_V_STRUCTS_H
#define OS_RISC_V_STRUCTS_H

typedef struct FileNameList {
    char name[128];
    struct FileNameList *next;
} FileNameList;

FileNameList* FileNameListCreate(char* name);

#endif //OS_RISC_V_STRUCTS_H
