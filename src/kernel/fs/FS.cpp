#include "IFS.h"
#include "../stdio.h"
#include "../posix/posix_structs.h"

IFS *fs;

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
    return f_unlink(path);
}

int FS::link(const char *path, int flag) {
    return 0;
}

int FS::mkdir(const char *path, int flag) {
    return f_mkdir(path);
}

int FS::close(const char *path) {
    auto &el = path_to_fs_el.get(path);
    int fr = f_close(&el.data.fil);
    path_to_fs_el.erase(path);
    return fr;
}

int FS::write(const char *path, char *buf, int count) {
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

FRESULT scan_files(
        char *path,        /* Start node to be scanned (***also used as work area***) */
        char buf[],
        int length
) {
    FRESULT res;
    DIR dir;
    FILINFO fno;
    size_t ino = 0;
    size_t offset = 0;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            size_t len = 2 + offsetof(struct linux_dirent64, d_name) + strlen(fno.fname);
            size_t off = (len + 7) / 8 * 8;
            if(offset + off >= length)break;
            auto* dirent64 = (linux_dirent64*) (buf + offset);
            dirent64->d_ino = ino++;
            dirent64->d_reclen = length;
            dirent64->d_off = off;
            strcpy(dirent64->d_name, fno.fname);
            if (fno.fattrib & AM_DIR) {
                dirent64->d_type = DT_DIR;
            } else {
                dirent64->d_type = DT_REG;
            }
            offset += off;
        }
        f_closedir(&dir);
    }
    return res;
}


int FS::read_dir(const char *path, char *buf, int len) {
    String p = path;
    return scan_files(p.c_str(), buf, 512);
}
