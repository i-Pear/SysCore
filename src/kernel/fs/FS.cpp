#include "IFS.h"
#include "../stdio.h"
#include "../posix/posix_structs.h"
#include "VFS.h"

VFS *fs;

class FSUtil {
public:
    static int open_mode_to_fatfs_mode(int flag) {
        int result = 0;
        if (flag & O_CREATE)result |= FA_CREATE_ALWAYS;
        if (flag & O_WRONLY)result |= FA_WRITE;
        if (flag & O_RDWR)result |= (FA_WRITE | FA_READ);
        if (flag == 0)result |= FA_READ;
        return result;
    }

    static int fatfs_mode_to_posix_mode(int mode) {
        if (mode & AM_DIR)return S_IFDIR;
        return S_IFREG;
    }

    static int add_flag(const char *path, int flag) {
        if (flag == 0) {
            flag |= O_RDWR;
        }
        if ((strlen(path) > 0 && path[strlen(path) - 1] == '/') || flag & O_DIRECTORY) {
            flag |= S_IFDIR;
        } else {
            flag |= S_IFREG;
        }
        return flag;
    }
};

int FS::init() {
    has_last_dir = 0;
    static FATFS fatfs;
    f_mount(&fatfs, "", 1);
    return 0;
}

int FS::open(const char *path, int flag) {
    flag = FSUtil::add_flag(path, flag);
    if (flag & S_IFREG) {
        FS_Data data{};
        int fr = f_open(&data.fil, path + 1, FSUtil::open_mode_to_fatfs_mode(flag));
        FS_Element el(data, true);
        path_to_fs_el.put(String(path), el);
        return fr;
    } else if (flag & S_IFDIR) {
        FS_Data data{};
        int fr = f_opendir(&data.dir, path + 1);;
        FS_Element el(data, false);
        path_to_fs_el.put(String(path), el);
        return fr;
    }
    return -1;
}

int FS::fstat(const char *path, kstat *stat) {
    FILINFO fileInfo;
    int fr = f_stat(path, &fileInfo);
    if (fr != FR_OK)return -1;
    stat->st_size = fileInfo.fsize;
    stat->st_mtime_sec = fileInfo.ftime;
    stat->st_mtime_nsec = fileInfo.ftime;
    stat->st_ctime_sec = fileInfo.ftime;
    stat->st_ctime_nsec = fileInfo.ftime;
    stat->st_atime_sec = fileInfo.ftime;
    stat->st_atime_nsec = fileInfo.ftime;
    stat->st_dev = 0;
    stat->st_ino = 0;
    stat->st_mode = FSUtil::fatfs_mode_to_posix_mode(fileInfo.fattrib);
    stat->st_nlink = 0;
    return 0;
}

int FS::unlink(const char *path) {
    return f_unlink(path);
}

int FS::mkdir(const char *path, int flag) {
    //TODO: mkdir syscall error
    return f_mkdir(path);
}

int FS::close(const char *path) {
    auto &el = path_to_fs_el.get(path);
    int fr = f_close(&el.data.fil);
    path_to_fs_el.erase(path);
    return fr;
}

int FS::write(const char *path, char *buf, int count) {
    // why qemu boom after delete this code?
    if (strcmp(path, "/dev/console") == 0) {
        for (int i = 0; i < count; i++) {
            putchar(buf[i]);
        }
        return count;
    }
    auto &el = path_to_fs_el.get(path);
    unsigned int result = 0;
    int fr = f_write(&el.data.fil, buf, count, &result);
    if (fr != FR_OK) return -1;
    return result;
}

int FS::read(const char *path, char *buf, int count) {
    unsigned int result = 0;
    auto &el = path_to_fs_el.get(path);
    int fr = f_read(&el.data.fil, buf, count, &result);
    if (fr != FR_OK)return -1;
    return result;
}

int FS::read_dir(const char *path, char *buff, int new_request) {
    FRESULT res;
    FILINFO fno;

    if (!has_last_dir || new_request != 0) {
        if (has_last_dir != 0) {
            f_closedir(&last_dir);
        }
        res = f_opendir(&last_dir, path);
        has_last_dir = 1;
        if (res != FR_OK) {
            f_closedir(&last_dir);
            return -1;
        }
    }
    res = f_readdir(&last_dir, &fno);
    if (res != FR_OK || fno.fname[0] == 0) {
        f_closedir(&last_dir);
        has_last_dir = 0;
        return 0;
    }
    auto *dirInfo = (DirInfo *) buff;
    if (fno.fattrib & AM_DIR) {
        dirInfo->fattrib = S_IFDIR;
    } else {
        dirInfo->fattrib = S_IFREG;
    }
    dirInfo->fdate = fno.fdate;
    dirInfo->ftime = fno.ftime;
    dirInfo->fsize = fno.fsize;
    strcpy(dirInfo->fname, fno.fname);
    return 1;
}

int FS::lseek(const char *path, int offset) {
    auto &el = path_to_fs_el.get(path);
    auto res = f_lseek(&el.data.fil, offset);
    if(res != FR_OK){
        return -1;
    }
    return 0;
}
