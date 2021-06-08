#ifndef OS_RISC_V_IFS_H
#define OS_RISC_V_IFS_H

#include "stddef.h"
#include "../../driver/fatfs/ff.h"
#include "../../lib/stl/map.h"
#include "FSMacro.h"

class IFS{
public:
    virtual int init() = 0;

    virtual int open(const char *path, int flag) = 0;

    virtual int read(const char *path, char buf[], int count) = 0;

    virtual int write(const char *path, char buf[], int count) = 0;

    virtual int close(const char *path) = 0;

    virtual int mkdir(const char *path, int flag) = 0;

    virtual int link(const char *path, int flag) = 0;

    virtual int unlink(const char *path) = 0;

    virtual int mount(const char *dist,const char *origin) = 0;

    virtual int umount(const char *dist) = 0;

    virtual int fstat(const char *path, size_t *result, int option) = 0;
};

class FS: public IFS{
private:

public:
    int init() override;

    int open(const char *path, int flag) override;

    int read(const char *path, char *buf, int count) override;

    int write(const char *path, char *buf, int count) override;

    int close(const char *path) override;

    int mkdir(const char *path, int flag) override;

    int link(const char *path, int flag) override;

    int unlink(const char *path) override;

    int mount(const char *dist, const char *origin) override;

    int umount(const char *dist) override;

    int fstat(const char *path, size_t *result, int option) override;
};



#endif //OS_RISC_V_IFS_H
