#ifndef OS_RISC_V_FILE_DESCRIBER_H
#define OS_RISC_V_FILE_DESCRIBER_H

#include "../driver/fatfs/ff.h"
#include "stdbool.h"
#include "stl.h"

enum File_Describer_Type{
    FILE_DESCRIBER_FILE,
    FILE_DESCRIBER_PIPE,
    FILE_DESCRIBER_DIR
};

enum File_Access_Type{
    FILE_ACCESS_READ=1,
    FILE_ACCESS_WRITE=2,
};

typedef struct {
    enum File_Describer_Type fileDescriberType;
    enum File_Access_Type fileAccessType;
    union{
        FIL fat32;
        DIR fat32_dir;
        int pipe_id;
    } data;
    char* dir_name;
} File_Describer;

#define FILE_DESCRIBER_ARRAY_LENGTH 100

extern File_Describer file_describer_array[];
extern bool file_describer_array_occupied[];

void init_file_describer();

int get_new_file_describer();

void erase_file_describer(int n);

#endif //OS_RISC_V_FILE_DESCRIBER_H
