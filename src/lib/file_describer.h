#ifndef OS_RISC_V_FILE_DESCRIBER_H
#define OS_RISC_V_FILE_DESCRIBER_H

#include "../driver/fatfs/ff.h"
#include "stdbool.h"
#include "stl.h"

enum File_Describer_Type{
    FILE_DESCRIBER_FILE,
    FILE_DESCRIBER_PIPE,
    FILE_DESCRIBER_DIR,
    FILE_DESCRIBER_REDIRECT
};

enum File_Access_Type{
    FILE_ACCESS_READ=1,
    FILE_ACCESS_WRITE=2,
};

typedef union{
    FIL fat32;
    DIR fat32_dir;
    int pipe_id;
    int redirect_fd;
} File_Describer_Data;

typedef union {
    char* dir_name;
} File_Describer_Extra_Data;

typedef struct {
    enum File_Describer_Type fileDescriberType;
    enum File_Access_Type fileAccessType;
    File_Describer_Data data;
    File_Describer_Extra_Data extraData;
} File_Describer;

#define FILE_DESCRIBER_ARRAY_LENGTH 100

extern File_Describer file_describer_array[];

/**
 * 初始化VFS
 */
void init_file_describer();

/**
 * 在fd处初始化一个文件描述符
 * @param fd
 * @param fileDescriberType：文件描述符类型
 * @param fileAccessType：文件访问权限
 * @param data：数据
 * @param extraData：额外数据
 */
void File_Describer_Create(
        int fd,
        enum File_Describer_Type fileDescriberType,
        enum File_Access_Type fileAccessType,
        File_Describer_Data data,
        File_Describer_Extra_Data extraData);

/**
 * fd引用计数加一
 * @param fd
 */
void File_Describer_Plus(int fd);

/**
 * fd引用计数减一
 * 如果fd是重定向文件描述符，那么包括此fd在内一直到源文件描述符的引用计数都会减一
 * 如果引用计数为0则释放该文件描述符
 * @param fd
 */
void File_Describer_Reduce(int fd);

/*
 *  私有接口
 */

int fd_write_to_file(int fd, char* buf, int count);

int fd_get_origin_fd(int fd);

int fd_search_a_empty_file_describer();

#endif //OS_RISC_V_FILE_DESCRIBER_H
