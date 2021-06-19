#ifndef OS_RISC_V_VFS_H
#define OS_RISC_V_VFS_H

#include "../../lib/stl/Trie.h"
#include "../../lib/stl/string.h"
#include "../../lib/stl/PathUtil.h"
#include "IFS.h"

class VFS {
public:
    IFS *fs;
    Trie<String, VFS *> tree;

#define VFS_ADAPTER(X, Y) auto paths = PathUtil::split(path); \
    auto p = tree.firstMatch(paths); \
    if(p == nullptr){ \
        return fs->X;\
    }\
    auto new_path = PathUtil::joinAbsolutePath(paths).c_str();          \
    int res = (*p)->Y;                                        \
    delete new_path;                                                          \
    return res;

    VFS(IFS* ifs) {
        init(ifs);
    }

    int init(IFS *ifs) {
        fs = ifs;
        return 0;
    }

    int open(const char *path, int flag) {
        VFS_ADAPTER(open(path, flag), open(new_path, flag));
    }

    int read(const char *path, char buf[], int count) {
        VFS_ADAPTER(read(path, buf, count), read(new_path, buf, count));
    }

    int write(const char *path, char buf[], int count) {
        VFS_ADAPTER(write(path, buf, count), write(new_path, buf, count));
    }

    int close(const char *path) {
        VFS_ADAPTER(close(path), close(new_path));
    }


    int fstat(const char *path, kstat* stat){
        VFS_ADAPTER(fstat(path, stat), fstat(new_path, stat));
    };

    int read_dir(const char* path, char buf[], int len){
        VFS_ADAPTER(read_dir(path, buf, len), read_dir(new_path, buf, len));
    };

    int mkdir(const char *path, int flag){
        VFS_ADAPTER(mkdir(path, flag), mkdir(new_path, flag));
    };

    int unlink(const char *path){
        VFS_ADAPTER(unlink(path), unlink(new_path));
    }

    int mount(const char *dist, const char *origin){
        VFS* vfs = new VFS(fs);
        tree.insert(PathUtil::split(dist), vfs);
        return 0;
    };

    int link(const char *path, int flag){
        // TODO implement
        return 0;
    };


    int umount(const char *dist){
        //TODO implement
        return 0;
    };
};

extern VFS* fs;

#endif //OS_RISC_V_VFS_H
