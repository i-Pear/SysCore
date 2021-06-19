#ifndef OS_RISC_V_VFS_H
#define OS_RISC_V_VFS_H

#include "../../lib/stl/Trie.h"
#include "../../lib/stl/string.h"
#include "../../lib/stl/PathUtil.h"
#include "IFS.h"

class VFS{
public:
    IFS* fs;
    Trie<String, VFS*> tree;

    VFS(){

    }



    int init(IFS* ifs){
        fs = ifs;
        return 0;
    }

    int open(const char *path, int flag){
        auto paths = PathUtil::split(path);
        auto p = tree.firstMatch(paths);
        if(p == nullptr){
            return fs->open(path, flag);
        }

        return 0;
    }

    int read(const char *path, char buf[], int count){
        return 0;
    }

    int write(const char *path, char buf[], int count){
        return 0;
    }

    int close(const char *path){
        return 0;
    }
};

#endif //OS_RISC_V_VFS_H
