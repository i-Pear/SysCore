#include "vfs.h"
#include "stl.h"

int (*vfs_init)() = NULL;

int (*vfs_open)(const char *path, int flag) = NULL;

unsigned int (*vfs_read)(const char *path, char buf[], int count) = NULL;

unsigned int (*vfs_write)(const char *path, char buf[], int count) = NULL;

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
int vfs_open_mode_to_fatfs_mode(int flag) {
    int result = 0;
    if (flag & O_CREATE)result |= FA_CREATE_NEW;
    if (flag & O_WRONLY)result |= FA_WRITE;
    if (flag & O_RDWR)result |= (FA_WRITE | FA_READ);
    if (flag == 0)result |= FA_READ;
    return result;
}

int vfs_add_flag(int flag){
    if(flag == 0){
        flag |= S_IFREG;
        flag |= O_RDWR;
    }
    return flag;
}

/// implement
int ff_init() {
    static FATFS fatfs;
    f_mount(&fatfs, "", 1);
    return 0;
}

// TODO: now only support open one file
static FIL fil;
static DIR dir;
static int is_opened = 0;

int ff_open(const char *path, int flag) {
    if (is_opened == 1) {
        panic("only support open one file\n");
    }
    flag = vfs_add_flag(flag);
    if (flag & S_IFREG) {
        is_opened = 1;
        int fr = f_open(&fil, path + 1, vfs_open_mode_to_fatfs_mode(flag));
        return fr;
    } else if (flag & S_IFDIR) {
        is_opened = 1;
        return f_opendir(&dir, path + 1);
    }
    return -1;
}

unsigned int ff_read(const char *path, char buf[], int count) {
    unsigned int result = 0;
    FRESULT fr = f_read(&fil, buf, count, &result);
    if (fr != FR_OK)return -1;
    return result;
}

unsigned int ff_write(const char *path, char buf[], int count) {
    if(strcmp(path, "/dev/console") == 0){
        for(int i = 0;i < count; i++){
            putchar(buf[i]);
        }
        return count;
    }
    unsigned int result = 0;
    FRESULT fr = f_write(&fil, buf, count, &result);
    if (fr != FR_OK) return -1;
    return result;
}

int ff_close(const char *path) {
    is_opened = 0;
    return f_close(&fil);
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
    }else{
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
