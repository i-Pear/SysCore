#ifndef OS_RISC_V_FILE_DESCRIBER_H
#define OS_RISC_V_FILE_DESCRIBER_H

#include "stdbool.h"
#include "../lib/stl/stl.h"
#include "fs/IFS.h"

enum File_Describer_Type {
    FILE_DESCRIBER_REGULAR,
    FILE_DESCRIBER_REDIRECT
};

enum File_Access_Type {
    FILE_ACCESS_READ = 1,
    FILE_ACCESS_WRITE = 2,
    FILE_ACCESS_RW = 3
};

typedef union {
    int redirect_fd;
} File_Describer_Data;

typedef struct {
    enum File_Describer_Type fileDescriberType;
    enum File_Access_Type fileAccessType;
    File_Describer_Data data;
    char *path;
} File_Describer;

// 最大文件描述符个数
#define FILE_DESCRIBER_ARRAY_LENGTH 120

extern File_Describer file_describer_array[];
extern int file_describer_array_occupied[];

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
        char *path);

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

char *File_Describer_Get_Path(int fd);

/*
 *  私有接口
 */

int fd_write_to_file(int fd, char *buf, int count);

int fd_get_origin_fd(int fd);

int fd_search_a_empty_file_describer();

#endif //OS_RISC_V_FILE_DESCRIBER_H