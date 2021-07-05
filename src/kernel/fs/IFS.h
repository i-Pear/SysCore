#ifndef OS_RISC_V_IFS_H
#define OS_RISC_V_IFS_H

#include "stddef.h"
#include "../../driver/fatfs/ff.h"
#include "../../lib/stl/map.h"
#include "FSMacro.h"
#include "../../lib/stl/string.h"
#include "../posix/posix_structs.h"

#define NOT_IMPLEMENT printf("%s not implement!\n", __FUNCTION__); panic("")

struct DirInfo {
    FSIZE_t fsize;            /* File size */
    WORD fdate;            /* Modified date */
    WORD ftime;            /* Modified time */
    int fattrib;        /* File attribute */
    TCHAR fname[FF_LFN_BUF + 1];    /* Primary file name */
};

class IFS {
public:
    virtual int init() {
        NOT_IMPLEMENT
        return 0;
    };

    virtual int open(const char *path, int flag) {
        NOT_IMPLEMENT
        return 0;
    };

    virtual int read(const char *path, char buf[], int count) {
        NOT_IMPLEMENT
        return 0;
    };

    virtual int write(const char *path, char buf[], int count) {
        NOT_IMPLEMENT
        return 0;
    };

    virtual int close(const char *path) {
        NOT_IMPLEMENT
        return 0;
    };

    virtual int mkdir(const char *path, int flag) {
        NOT_IMPLEMENT
        return 0;
    };

    virtual int unlink(const char *path) {
        NOT_IMPLEMENT
        return 0;
    };

    virtual int fstat(const char *path, kstat *stat) {
        NOT_IMPLEMENT
        return 0;
    };

    virtual int lseek(const char *path, size_t offset, int whence) {
        NOT_IMPLEMENT
        return 0;
    };

    /**
     * read directory entry
     * @param path: directory path
     * @param buff: a DirInfo Object buffer
     * @param new_request: is this is a new request
     * @return -1 when error, 0 when end, 1 when ok
     */
    virtual int read_dir(const char *path, char buff[sizeof(DirInfo)], int new_request) {
        NOT_IMPLEMENT
        return 0;
    };

    virtual int pipe(const char *read_path, const char *write_path, int flags, int unused_param) {
        NOT_IMPLEMENT
        return 0;
    }
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

    int lseek(const char *path, size_t offset, int whence) override;
};


class StdoutFs : public IFS {
public:
    int init() override {
        return 0;
    }

    int write(const char *path, char *buf, int count) override {
        for (auto i = 0; i < count; i++) {
            putchar(buf[i]);
        }
        return count;
    }
};

class PipeFs: public IFS{
private:
    Map<String, List<unsigned char> *> read_pair, write_pair;
public:
    int init() override {
        return IFS::init();
    }

    int read(const char *path, char *buf, int count) override {
        auto *read_buff = read_pair.get(String(path));
        if(read_buff == nullptr)return -1;
        int cur = 0;
        for (auto i = 0; i < count; i++) {
            if (!read_buff->is_empty()) {
                buf[cur++] = read_buff->start->data;
                read_buff->pop_front();
            } else {
                // TODO: 此处直接实现为非阻塞，如果没有足够的数据会直接返回
                return cur;
            }
        }
        return cur;
    }

    int write(const char *path, char *buf, int count) override {
        auto *write_buff = write_pair.get(String(path));
        if(write_buff == nullptr)return -1;
        for (auto i = 0; i < count; i++) {
            write_buff->push_back(buf[i]);
        }
        return count;
    }

    int close(const char *path) override {
        if (read_pair.exists(String(path))) {
            auto *buff_list = read_pair.get(String(path));
            delete buff_list;
            read_pair.erase(String(path));
        }
        if (write_pair.exists(String(path))) {
            auto *buff_list = write_pair.get((String(path)));
            delete buff_list;
            write_pair.erase(String(path));
        }
        return 0;
    }

    int pipe(const char *read_path, const char *write_path, int flags, int unused_param) override {
        auto *buff = new List<unsigned char>();
        read_pair.put(String(read_path), buff);
        write_pair.put(String(write_path), buff);
        return 0;
    }
};

#endif //OS_RISC_V_IFS_H
