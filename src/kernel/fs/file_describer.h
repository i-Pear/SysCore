#ifndef OS_RISC_V_FILE_DESCRIBER_H
#define OS_RISC_V_FILE_DESCRIBER_H

#include "stdbool.h"
#include "../../lib/stl/stl.h"
#include "VFS.h"
#include "../../lib/stl/RefCountPtr.h"
#include "../../lib/stl/string.h"
#include "../../lib/stl/map.h"

#define FILE_DESCRIBER_ARRAY_LENGTH 120

class OpenedFile{
public:
    explicit OpenedFile(const String& path):path_(path){}
    virtual ~OpenedFile(){
        printf("[OpenedFile] close %s\n", path_.c_str());
        fs->close(path_.c_str());
    }
    String& GetPath(){
        return path_;
    }
    char* GetCStylePath(){
        return path_.c_str();
    }
private:
    String path_;
};

enum class FILE_ACCESS_TYPE{
    READ = 1, WRITE = 2, RW = 3
};

class FileDescriber{
public:
    FileDescriber(const RefCountPtr<OpenedFile>& file, FILE_ACCESS_TYPE fileAccessType): file_access_type_(fileAccessType){
        file_ = file;
    }

    RefCountPtr<OpenedFile>& GetFile(){
        return file_;
    }

    FILE_ACCESS_TYPE GetAccessType(){
        return file_access_type_;
    }
private:
    FILE_ACCESS_TYPE file_access_type_;
    RefCountPtr<OpenedFile> file_;
};

extern FileDescriber* fd_array[FILE_DESCRIBER_ARRAY_LENGTH];

class FD{
public:
    static void InitializeFileDescriber(){
        for(int i = 0;i < FILE_DESCRIBER_ARRAY_LENGTH; i++){
            fd_array[i] = nullptr;
        }
        RefCountPtr<OpenedFile> stdout(new OpenedFile("/dev/console"));
        RefCountPtr<OpenedFile> stdin(new OpenedFile(""));
        fd_array[0] = new FileDescriber(stdin, FILE_ACCESS_TYPE::READ);
        fd_array[1] = new FileDescriber(stdout, FILE_ACCESS_TYPE::WRITE);
        fd_array[2] = new FileDescriber(stdout, FILE_ACCESS_TYPE::WRITE);
    }

    static int OpenNewFile(const char* path, int flag, int fd){
        printf("[FD] open file %s\n", path);
        int res = fs->open(path, flag);
        if(res == FR_OK){
            RefCountPtr<OpenedFile> file(new OpenedFile(path));
            Check(fd);
            fd_array[fd] = new FileDescriber(file, FILE_ACCESS_TYPE::RW);
        }
        return res;
    }

    static int TryCloseFile(int fd){
        if(!Exists(fd)){
            panic("[FD] Try close un exists fd")
            return -1;
        }
//        printf("[FD] try close file %s\n", GetFile(fd)->GetCStylePath());
        Erase(fd);
    }

    static RefCountPtr<OpenedFile>& GetFile(int fd){
        return fd_array[fd]->GetFile();
    }

    static void CopyFd(int source_fd, int target_fd){
        if(!Exists(source_fd)){
            printf("[FD] fd %d not exists!\n", source_fd);
            panic("")
        }
        if(Exists(target_fd)){
            Erase(target_fd);
        }
        fd_array[target_fd] = new FileDescriber(fd_array[source_fd]->GetFile(), fd_array[source_fd]->GetAccessType());
    }

    static int FindUnUsedFd(){
        for(int i = 0;i < FILE_DESCRIBER_ARRAY_LENGTH; i++){
            if(!Exists(i)){
                return i;
            }
        }
        panic("can't find unused fd");
        return -1;
    }

    static int FindUnusedFdBiggerOrEqual(int given_fd){
        for(int i = given_fd;i < FILE_DESCRIBER_ARRAY_LENGTH; i++){
            if(!Exists(i)){
                return i;
            }
        }
        panic("can't find unused fd");
        return -1;
    }
private:
    static void Check(int fd){
        if(fd_array[fd] != nullptr){
            printf("[FD] fd %d is used\n", fd);
            panic("")
        }
    }

    static bool Exists(int fd){
        return fd_array[fd] != nullptr;
    }

    static void Erase(int fd){
        delete fd_array[fd];
        fd_array[fd] = nullptr;
    }
};

#endif //OS_RISC_V_FILE_DESCRIBER_H