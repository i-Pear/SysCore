#include "file_describer.h"

// 保留文件描述符个数 fd = 1：stdout
#define FILE_DESCRIBER_RESERVED_FD_COUNT 3

File_Describer file_describer_array[FILE_DESCRIBER_ARRAY_LENGTH];
int file_describer_array_occupied[FILE_DESCRIBER_ARRAY_LENGTH];

int fd_get_origin_fd(int fd){
    if(fd >= FILE_DESCRIBER_ARRAY_LENGTH || fd <= 0){
        return fd;
    }
    if(file_describer_array[fd].fileDescriberType == FILE_DESCRIBER_REDIRECT){
        return fd_get_origin_fd(file_describer_array[fd].data.redirect_fd);
    }
    return fd;
}

void File_Describer_Create(
        int fd,
        enum File_Describer_Type fileDescriberType,
        enum File_Access_Type fileAccessType,
        File_Describer_Data data,
        char* path) {
    File_Describer_Plus(fd);
    file_describer_array[fd].fileDescriberType = fileDescriberType;
    file_describer_array[fd].fileAccessType = fileAccessType;
    file_describer_array[fd].data = data;
    file_describer_array[fd].path = (char *)k_malloc(strlen(path) + 1);
    strcpy(file_describer_array[fd].path, path);
}

void File_Describer_Plus(int fd){
    file_describer_array_occupied[fd] += 1;
}

void File_Describer_Reduce(int fd){
    assert(fd >= 0)
    assert(fd < FILE_DESCRIBER_ARRAY_LENGTH)
    if(file_describer_array[fd].fileDescriberType == FILE_DESCRIBER_REDIRECT){
        file_describer_array_occupied[fd] -= 1;
        assert(file_describer_array_occupied[fd] >= 0)
        File_Describer_Reduce(file_describer_array[fd].data.redirect_fd);
        return;
    }
    file_describer_array_occupied[fd] -= 1;
    assert(file_describer_array_occupied[fd] >= 0)
    // 如果引用计数为0则释放path空间
    if(file_describer_array_occupied[fd] == 0){
        //printf("//// close fd %d\n", fd);
        vfs_close(file_describer_array[fd].data.inode);
        k_free((size_t)file_describer_array[fd].path);
    }
}

void init_file_describer() {
    memset(file_describer_array_occupied, 0, sizeof(file_describer_array_occupied));
    for (int i = 0; i < FILE_DESCRIBER_RESERVED_FD_COUNT; ++i) {
        file_describer_array_occupied[i] = 1;
    }
    Inode * stdout_inode = vfs_open("/dev/console", O_CREATE, S_IFCHR);
    if(stdout_inode == null){
        panic("stdout init error")
    }
    File_Describer_Data data = {.inode = stdout_inode};
    File_Describer_Create(1, FILE_DESCRIBER_REGULAR, FILE_ACCESS_WRITE, data, "/dev/console");
}

int fd_search_a_empty_file_describer() {
    for (int i = 0; i < FILE_DESCRIBER_ARRAY_LENGTH; i++) {
        if (file_describer_array_occupied[i] == 0) {
            return i;
        }
    }
    panic("no empty file describer");
    return -1;
}