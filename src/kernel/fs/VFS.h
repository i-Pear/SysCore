#ifndef OS_RISC_V_VFS_H
#define OS_RISC_V_VFS_H

#include "../../lib/stl/Trie.h"
#include "../../lib/stl/string.h"
#include "../../lib/stl/PathUtil.h"
#include "IFS.h"

extern size_t global_ino;

class File {
public:
    IFS *fs;
    size_t ino;
    String name{};
    File *parent{}, *first_child{}, *previous{}, *next{};

    static void init() {
        global_ino = 0;
    }

    void deleteTree(File *file) {
        if (file == nullptr)return;
        if (file->next)deleteTree(file->next);
        if (file->first_child)deleteTree(file->first_child);
        delete file;
    }

    explicit File(const String &fileName, File *parentFile, IFS *ifs) : name(fileName), parent(parentFile), fs(ifs),
                                                                        ino(global_ino++) {}

    explicit File(const String &fileName, IFS *ifs) : name(fileName), fs(ifs), ino(global_ino++) {}

    void setNext(File *nextFile) {
        next = nextFile;
        if (nextFile != nullptr) {
            nextFile->previous = this;
            nextFile->parent = this->parent;
        }
    }

    void deleteNext() {
        if (next != nullptr) {
            auto *t = next;
            if (t->next) {
                t->next->previous = this;
            }
            this->next = t->next;
            delete t;
        }
    }

    bool appendChild(File *childFile) {
        if (first_child == nullptr) {
            first_child = childFile;
            if (childFile != nullptr) {
                childFile->parent = this;
            }
            return true;
        }
        auto *p = first_child;
        while (p->next) {
            if (p->name == childFile->name) {
                return false;
            }
            p = p->next;
        }
        p->setNext(childFile);
        return true;
    }

    void deleteChild(File *childFile) {
        if (childFile == nullptr)return;
        auto *p = first_child;
        while (p) {
            if (p->name == childFile->name) {
                if (p->previous) {
                    p->previous->next = p->next;
                } else if (p->parent) {
                    p->parent->first_child = nullptr;
                }
                if (p->next) {
                    p->next->previous = p->previous;
                }
                deleteTree(p->first_child);
                delete p;
                break;
            }
            p = p->next;
        }
    }

    File *search(const List<String> &path) {
        auto *p = path.start;
        File *searchObj = this;
        while (p) {
            auto &filename = p->data;
            bool isFind = false;
            while (searchObj) {
                if (searchObj->name == filename) {
                    isFind = true;
                    break;
                }
                searchObj = searchObj->next;
            }
            if (!isFind)return nullptr;
            if (p->next == nullptr) {
                return searchObj;
            }
            searchObj = searchObj->first_child;
            p = p->next;
        }
        return nullptr;
    }

    File *search(const String &path) {
        return search(PathUtil::split(path));
    }

    File *innerSearch(File *file, size_t i_no) {
        if (file == nullptr)return nullptr;
        if (file->ino == i_no)return file;
        auto *res = innerSearch(file->next, i_no);
        if (res != nullptr)return res;
        return innerSearch(file->first_child, i_no);
    }

    File *search(size_t i_no) {
        return innerSearch(this, i_no);
    }
};

class VFS {
#define VFS_ADAPTER(x) \
    auto* file = search(root, path);\
    if(file == nullptr){\
        return -1;\
    }\
    return file->fs->x;
public:
    IFS *fs;
    File *root;

    VFS(IFS *ifs) {
        init(ifs);
    }

    void scan(const String& str, File *dir) {
//        printf("%s\n", str.c_str());
        static DirInfo dirInfo;
        int res;
        res = fs->read_dir(str.c_str(), (char *) &dirInfo, 1);
        if (res == -1) {
            return;
        }
        dir->appendChild(new File(dirInfo.fname, fs));
        while (res != 0){
            res = fs->read_dir(str.c_str(), (char *) &dirInfo, 0);
            if (res == -1) {
                return;
            }
            dir->appendChild(new File(dirInfo.fname, fs));
        }
        auto *p = dir->first_child;
        while (p){
            if(str == "/"){
                scan(str + p->name, p);
            }else{
                scan(str + "/" + p->name, p);
            }
            p = p->next;
        }
    }

    int init(IFS *ifs) {
        fs = ifs;
        root = new File("/", fs);
        String str = "/";
        scan(str, root);
        return 0;
    }

    static File* search(File* file, const char* path){
        if(path == nullptr){
            return nullptr;
        }
        if(strcmp(path, "/") == 0){
            return file;
        }
        return file->first_child->search(path);
    }

    int open(const char *path, int flag) {
        VFS_ADAPTER(open(path, flag))
    }

    int read(const char *path, char buf[], int count) {
        if(path == nullptr)return -1;
        VFS_ADAPTER(read(path, buf, count))
    }

    int write(const char *path, char buf[], int count) {
        VFS_ADAPTER(write(path, buf, count))
    }

    int close(const char *path) {
        VFS_ADAPTER(close(path))
    }


    int fstat(const char *path, kstat *stat) {
        VFS_ADAPTER(fstat(path, stat))
    };

    int read_dir(const char *path, char buf[], int len) {
        // un implement
        return 0;
    };

    int mkdir(const char *path, int flag) {
        VFS_ADAPTER(mkdir(path, flag))
//        return 0;
    };

    int unlink(const char *path) {
        return 0;
    }

    int mount(const char *dist, const char *origin) {
        return 0;
    };

    int link(const char *path, int flag) {
        // TODO implement
        return 0;
    };

    int umount(const char *dist) {
        //TODO implement
        return 0;
    };
};

class TestFile {
public:
    TestFile() {
        File *root = new File("/", nullptr, nullptr);
        root->appendChild(new File("a", nullptr));
        root->appendChild(new File("b", nullptr));
        root->first_child->appendChild(new File("c", nullptr));
        root->first_child->appendChild(new File("d", nullptr));

        auto list = PathUtil::split("/a/c");
        auto *c = root->first_child->search(list);
        auto c_ino = c->ino;
        assert(c != nullptr && c->name == "c");

        auto *ci = root->search(c_ino);
        assert(ci->name == c->name);

        auto *a = root->first_child->search(PathUtil::split("a"));
        assert(a != nullptr);
        a->deleteChild(c);


        c = root->first_child->search(PathUtil::split("/a/c"));
        assert(c == nullptr);

        auto *b = root->first_child->search(PathUtil::split("b"));
        assert(b != nullptr);

        a->deleteNext();

        b = root->first_child->search(PathUtil::split("b"));
        assert(b == nullptr);
    }
};

extern VFS *fs;

#endif //OS_RISC_V_VFS_H
