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
        fs->close(file_describer_array[fd].path);
        k_free((size_t)file_describer_array[fd].path);
    }
}

void init_file_describer() {
    memset(file_describer_array_occupied, 0, sizeof(file_describer_array_occupied));
    for (int i = 0; i < FILE_DESCRIBER_RESERVED_FD_COUNT; ++i) {
        file_describer_array_occupied[i] = 1;
    }
    File_Describer_Data fakeData = {.redirect_fd = 0};
    File_Describer_Create(1, FILE_DESCRIBER_REGULAR, FILE_ACCESS_WRITE, fakeData, "/dev/console");
    File_Describer_Plus(1);
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

char *File_Describer_Get_Path(int fd) {
    assert(fd >= 0 && fd < FILE_DESCRIBER_ARRAY_LENGTH);
    return file_describer_array[fd].path;
}