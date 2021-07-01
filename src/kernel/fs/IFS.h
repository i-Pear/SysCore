#ifndef OS_RISC_V_IFS_H
#define OS_RISC_V_IFS_H

#include "stddef.h"
#include "../../driver/fatfs/ff.h"
#include "../../lib/stl/map.h"
#include "FSMacro.h"
#include "../../lib/stl/string.h"
#include "../posix/posix_structs.h"

#define NOT_IMPLEMENT printf("%s not implement!\n", __FUNCTION__); panic("");

struct DirInfo {
    FSIZE_t fsize;            /* File size */
    WORD fdate;            /* Modified date */
    WORD ftime;            /* Modified time */
    int fattrib;        /* File attribute */
    TCHAR fname[FF_LFN_BUF + 1];    /* Primary file name */
};

class IFS {
public:
    virtual int init() = 0;

    virtual int open(const char *path, int flag) = 0;

    virtual int read(const char *path, char buf[], int count) = 0;

    virtual int write(const char *path, char buf[], int count) = 0;

    virtual int close(const char *path) = 0;

    virtual int mkdir(const char *path, int flag) = 0;

    virtual int unlink(const char *path) = 0;

    virtual int fstat(const char *path, kstat *stat) = 0;

    virtual int lseek(const char* path, int offset) = 0;

    /**
     * read directory entry
     * @param path: directory path
     * @param buff: a DirInfo Object buffer
     * @param new_request: is this is a new request
     * @return -1 when error, 0 when end, 1 when ok
     */
    virtual int read_dir(const char *path, char buff[sizeof(DirInfo)], int new_request) = 0;
};


class FS : public IFS {
private:
    union FS_Data {
        FIL fil;
        DIR dir;
    };

    class FS_Element {
    public:
        explicit FS_Element() = default;

        FS_Element(FS_Data fsData, bool isFile) : is_file(isFile), data(fsData) {}

        FS_Data data{};
        bool is_file{};
    };

    Map<String, FS_Element> path_to_fs_el;

    DIR last_dir;
    int has_last_dir;
public:
    int init() override;

    int open(const char *path, int flag) override;

    int read(const char *path, char *buf, int count) override;

    int write(const char *path, char *buf, int count) override;

    int close(const char *path) override;

    int mkdir(const char *path, int flag) override;

    int unlink(const char *path) override;

    int fstat(const char *path, kstat *stat) override;

    int read_dir(const char *path, char *buff, int new_request) override;

    int lseek(const char *path, int offset) override;
};


class StdoutFs : public IFS {
public:
    int init() override {
        return 0;
    }

    int open(const char *path, int flag) override {
        NOT_IMPLEMENT
        return 0;
    }

    int read(const char *path, char *buf, int count) override {
        NOT_IMPLEMENT
        return 0;
    }

    int write(const char *path, char *buf, int count) override {
         for(auto i = 0;i < count; i++){
             putchar(buf[i]);
         }
        return count;
    }

    int close(const char *path) override {
        NOT_IMPLEMENT
        return 0;
    }

    int mkdir(const char *path, int flag) override {
        NOT_IMPLEMENT
        return 0;
    }

    int unlink(const char *path) override {
        NOT_IMPLEMENT
        return 0;
    }

    int fstat(const char *path, kstat *stat) override {
        NOT_IMPLEMENT
        return 0;
    }

    int read_dir(const char *path, char *buff, int new_request) override {
        NOT_IMPLEMENT
        return 0;
    }

    int lseek(const char *path, int offset) override {
        return 0;
    }
};

#endif //OS_RISC_V_IFS_H
