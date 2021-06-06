#include "vfs.h"
#include "stl.h"

int (*vfs_init)() = NULL;

int (*vfs_open)(const char *path, int flag) = NULL;

int (*vfs_read)(const char *path, char buf[], int count) = NULL;

int (*vfs_write)(const char *path, char buf[], int count) = NULL;

int (*vfs_close)(const char *path) = NULL;

int (*vfs_mkdir)(const char *path, int flag) = NULL;

int (*vfs_link)(const char *path, int flag) = NULL;

int (*vfs_unlink)(const char *path) = NULL;

int (*vfs_mount)(const char *dist, char *origin) = NULL;

int (*vfs_umount)(const char *dist) = NULL;

int (*vfs_fstat)(const char *path, size_t *result, int option) = NULL;

// fatfs implement
#include "../driver/fatfs/ff.h"

/// utils functions
void ff_copy_path(const char *path, int empty);

int vfs_open_mode_to_fatfs_mode(int flag) {
    int result = 0;
    if (flag & O_CREATE)result |= FA_CREATE_ALWAYS;
    if (flag & O_WRONLY)result |= FA_WRITE;
    if (flag & O_RDWR)result |= (FA_WRITE | FA_READ);
    if (flag == 0)result |= FA_READ;
    return result;
}

int vfs_add_flag(char* path, int flag) {
    if (flag == 0) {
        flag |= O_RDWR;
    }
    if((strlen(path) > 0 && path[strlen(path) - 1] == '/')|| flag & O_DIRECTORY){
        flag |= S_IFDIR;
    }else{
        flag |= S_IFREG;
    }
    return flag;
}

#define ff_get_file_id int id = ff_find_file(path);\
if(id < 0 || ff_list[id].is_file != 1)return -1;

/// implement
#define FF_ARRAY_LENGTH 10
struct FF_ELEMENT {
    char *path;
    union {
        FIL fil;
        DIR dir;
    } data;
    int is_file;
    int used;
};
struct FF_ELEMENT ff_list[FF_ARRAY_LENGTH];

int ff_init() {
    for (int i = 0; i < FF_ARRAY_LENGTH; i++) {
        struct FF_ELEMENT element = {.used = 0};
        ff_list[i] = element;
    }
    static FATFS fatfs;
    f_mount(&fatfs, "", 1);
    return 0;
}

int ff_find_empty() {
    for (int i = 0; i < FF_ARRAY_LENGTH; i++) {
        if (ff_list[i].used == 0) {
            ff_list[i].used = 1;
            return i;
        }
    }
    printf("only support open %d file\n", FF_ARRAY_LENGTH);
    panic("");
    return -1;
}

int ff_find_file(const char *path) {
    for (int i = 0; i < FF_ARRAY_LENGTH; i++) {
        if (ff_list[i].used == 1) {
            if (strcmp(ff_list[i].path, path) == 0) {
                return i;
            }
        }
    }
    return -1;
}


int ff_open(const char *path, int flag) {
    int empty = ff_find_empty();
    flag = vfs_add_flag(path, flag);
    if (flag & S_IFREG) {
        ff_list[empty].is_file = 1;
        ff_copy_path(path, empty);
        int fr = f_open(&ff_list[empty].data.fil, path + 1, vfs_open_mode_to_fatfs_mode(flag));
        return fr;
    } else if (flag & S_IFDIR) {
        ff_list[empty].is_file = 0;
        ff_copy_path(path, empty);
        return f_opendir(&ff_list[empty].data.dir, path + 1);
    }
    return -1;
}

void ff_copy_path(const char *path, int empty) {
    ff_list[empty].path = k_malloc(sizeof(path) + 1);
    strcpy(ff_list[empty].path, path);
}

int ff_read(const char *path, char buf[], int count) {
    unsigned int result = 0;
    ff_get_file_id
    FRESULT fr = f_read(&ff_list[id].data.fil, buf, count, &result);
    if (fr != FR_OK)return -1;
    return (int) result;
}

int ff_write(const char *path, char buf[], int count) {
    if (strcmp(path, "/dev/console") == 0) {
        for (int i = 0; i < count; i++) {
            putchar(buf[i]);
        }
        return count;
    }
    ff_get_file_id
    unsigned int result = 0;
    FRESULT fr = f_write(&ff_list[id].data.fil, buf, count, &result);
    if (fr != FR_OK) return -1;
    return (int) result;
}

int ff_close(const char *path) {
    ff_get_file_id
    k_free(ff_list[id].path);
    ff_list[id].used = 0;
    return f_close(&ff_list[id].data.fil);
}

int ff_mkdir(const char *path, int flag) {
    return f_mkdir(path);
}

int ff_unlink(const char *path) {
    return f_unlink(path);
}

int ff_stat(const char *path, size_t *result, int option) {
    FILINFO filinfo;
    FRESULT fr = f_stat(path, &filinfo);
    if (fr != FR_OK)return -1;
    if (option & FSTAT_FILE_SIZE) {
        *result = filinfo.fsize;
    } else {
        printf("ff_stat don't support option %d\n", option);
        panic("")
    }
    return 0;
}


void vfs_register() {
    vfs_init = ff_init;
    vfs_open = ff_open;
    vfs_fstat = ff_stat;
    vfs_unlink = ff_unlink;
    vfs_read = ff_read;
    vfs_write = ff_write;
    vfs_mkdir = ff_mkdir;
    vfs_close = ff_close;
}
