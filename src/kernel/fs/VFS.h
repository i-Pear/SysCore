#ifndef OS_RISC_V_VFS_H
#define OS_RISC_V_VFS_H

#include "../../lib/stl/Trie.h"
#include "../../lib/stl/string.h"
#include "../../lib/stl/PathUtil.h"
#include "IFS.h"

extern size_t global_ino;

class VFS {
public:
    IFS *fs;
    Trie<String, VFS *> tree;

#define VFS_ADAPTER(X, Y) \
    auto paths = PathUtil::split(path); \
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
        auto list = PathUtil::split(dist);
        tree.insert(list, vfs);
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

class File{
public:
    IFS* fs;
    size_t ino;
    String name{};
    File* parent{}, *first_child{}, *previous{}, *next{};

    static void init(){
        global_ino = 0;
    }

    void deleteTree(File* file){
        if(file == nullptr)return;
        if(file->next)deleteTree(file->next);
        if(file->first_child)deleteTree(file->first_child);
        delete file;
    }

    explicit File(const String& fileName, File* parentFile, IFS* ifs): name(fileName), parent(parentFile), fs(ifs), ino(global_ino++){}
    explicit File(const String& fileName, IFS* ifs): name(fileName), fs(ifs), ino(global_ino++){}

    void setNext(File* nextFile){
        next = nextFile;
        if(nextFile != nullptr){
            nextFile->previous = this;
            nextFile->parent = this->parent;
        }
    }

    void deleteNext(){
        if(next != nullptr){
            auto* t = next;
            if(t->next){
                t->next->previous = this;
            }
            this->next = t->next;
            delete t;
        }
    }

    bool appendChild(File* childFile){
        if(first_child == nullptr){
            first_child = childFile;
            if(childFile != nullptr){
                childFile->parent = this;
            }
            return true;
        }
        auto *p = first_child;
        while (p->next){
            if(p->name == childFile->name){
                return false;
            }
            p = p->next;
        }
        p->setNext(childFile);
        return true;
    }

    void deleteChild(File* childFile){
        if(childFile == nullptr)return;
        auto *p = first_child;
        while (p){
            if(p->name == childFile->name){
                if(p->previous){
                    p->previous->next = p->next;
                }else if(p->parent){
                    p->parent->first_child = nullptr;
                }
                if(p->next){
                    p->next->previous = p->previous;
                }
                deleteTree(p->first_child);
                delete p;
                break;
            }
            p = p->next;
        }
    }

    File* search(const List<String>& path){
        auto* p = path.start;
        File* searchObj = this;
        while (p){
            auto& filename = p->data;
            bool isFind = false;
            while (searchObj){
                if(searchObj->name == filename){
                    isFind = true;
                    break;
                }
                searchObj = searchObj->next;
            }
            if(!isFind)return nullptr;
            if(p->next == nullptr){
                return searchObj;
            }
            searchObj = searchObj->first_child;
            p = p->next;
        }
        return nullptr;
    }

    File* search(const String& path){
        return search(PathUtil::split(path));
    }

    File* innerSearch(File* file, size_t i_no){
        if(file == nullptr)return nullptr;
        if(file->ino == i_no)return file;
        auto* res = innerSearch(file->next, i_no);
        if(res != nullptr)return res;
        return innerSearch(file->first_child, i_no);
    }

    File* search(size_t i_no){
        return innerSearch(this, i_no);
    }
};

class TestFile{
public:
    TestFile(){
        File* root = new File("/", nullptr, nullptr);
        root->appendChild(new File("a",nullptr));
        root->appendChild(new File("b", nullptr));
        root->first_child->appendChild(new File("c", nullptr));
        root->first_child->appendChild(new File("d", nullptr));

        auto list = PathUtil::split("/a/c");
        auto* c = root->first_child->search(list);
        auto c_ino = c->ino;
        assert(c != nullptr && c->name == "c");

        auto* ci = root->search(c_ino);
        assert(ci->name == c->name);

        auto * a = root->first_child->search(PathUtil::split("a"));
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

extern VFS* fs;

#endif //OS_RISC_V_VFS_H
