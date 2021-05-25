#include "file_describer.h"

// 最大文件描述符个数
#define FILE_DESCRIBER_ARRAY_LENGTH 100
// 保留文件描述符个数 fd = 1：stdout
#define FILE_DESCRIBER_RESERVED_FD_COUNT 3

File_Describer file_describer_array[FILE_DESCRIBER_ARRAY_LENGTH];
int file_describer_array_occupied[FILE_DESCRIBER_ARRAY_LENGTH];

int fd_get_origin_fd(int fd){
    assert(fd >= 0)
    assert(fd < FILE_DESCRIBER_ARRAY_LENGTH)
    if(file_describer_array[fd].fileDescriberType == FILE_DESCRIBER_REDIRECT){
        return fd_get_origin_fd(fd);
    }
    return fd;
}

void File_Describer_Create(
        int fd,
        enum File_Describer_Type fileDescriberType,
        enum File_Access_Type fileAccessType,
        File_Describer_Data data,
        File_Describer_Extra_Data extraData) {
    File_Describer_Plus(fd);
    file_describer_array[fd].fileDescriberType = fileDescriberType;
    file_describer_array[fd].fileAccessType = fileAccessType;
    file_describer_array[fd].data = data;
    if(fileDescriberType == FILE_DESCRIBER_DIR && extraData.dir_name){
        int len = strlen(extraData.dir_name);
        file_describer_array[fd].extraData.dir_name = (char*) k_malloc(len + 1);
        strcpy(file_describer_array[fd].extraData.dir_name, extraData.dir_name);
    }
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
    // 如果引用计数为0则释放文件夹名空间
    if(file_describer_array_occupied[fd] == 0){
        if(file_describer_array[fd].fileDescriberType == FILE_DESCRIBER_DIR){
            if(file_describer_array[fd].extraData.dir_name){
                k_free((size_t)file_describer_array[fd].extraData.dir_name);
            }
        }
    }
}

void init_file_describer() {
    memset(file_describer_array_occupied, 0, sizeof(file_describer_array_occupied));
    for (int i = 0; i < FILE_DESCRIBER_RESERVED_FD_COUNT; ++i) {
        file_describer_array_occupied[i] = 1;
    }
    // 初始化stdout
    File_Describer_Data fakeData = {.redirect_fd = 0};
    File_Describer_Extra_Data fakeExtraData = {.dir_name = null};
    File_Describer_Create(1, FILE_DESCRIBER_FILE, FILE_ACCESS_WRITE, fakeData, fakeExtraData);
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

int fd_write_to_file(int fd, char* buf, int count){
    if(file_describer_array_occupied[fd] <= 0){
        printf("error fd: %d!\n", fd);
        panic("")
    }
    if(file_describer_array[fd].fileDescriberType == FILE_DESCRIBER_REDIRECT){
        return fd_write_to_file(file_describer_array[fd].data.redirect_fd, buf, count);
    }else{
        if(fd == 1){
            for (int i = 0; i < count; i++)putchar(buf[i]);
            return count;
        }else if(fd > 2){
            uint32_t write_bytes = 0;
            FRESULT result = f_write(&file_describer_array[fd].data.fat32, buf, count, &write_bytes);
            if (result != FR_OK) {
                return -1;
            } else {
                return (int)write_bytes;
            }
        }else{
            printf("error fd %d\n", fd);
            panic("")
        }
    }
    return -1;
}