#include "IFS.h"
class FSUtil{
    static int open_mode_to_fatfs_mode(int flag) {
        int result = 0;
        if (flag & O_CREATE)result |= FA_CREATE_ALWAYS;
        if (flag & O_WRONLY)result |= FA_WRITE;
        if (flag & O_RDWR)result |= (FA_WRITE | FA_READ);
        if (flag == 0)result |= FA_READ;
        return result;
    }
    static int add_flag(const char* path, int flag) {
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
};

int FS::init() {
    return 0;
}

int FS::open(const char *path, int flag) {
    return 0;
}

int FS::fstat(const char *path, size_t *result, int option) {
    return 0;
}

int FS::umount(const char *dist) {
    return 0;
}

int FS::mount(const char *dist, const char *origin) {
    return 0;
}

int FS::unlink(const char *path) {
    return 0;
}

int FS::link(const char *path, int flag) {
    return 0;
}

int FS::mkdir(const char *path, int flag) {
    return 0;
}

int FS::close(const char *path) {
    return 0;
}

int FS::write(const char *path, char *buf, int count) {
    return 0;
}

int FS::read(const char *path, char *buf, int count) {
    return 0;
}