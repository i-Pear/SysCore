#ifndef OS_RISC_V_FILE_DESCRIBER_H
#define OS_RISC_V_FILE_DESCRIBER_H

#include "stdbool.h"
#include "../../lib/stl/stl.h"
#include "VFS.h"
#include "../../lib/stl/RefCountPtr.h"
#include "../../lib/stl/string.h"
#include "../../lib/stl/map.h"
#include "FastPipe.h"

#define FILE_DESCRIBER_ARRAY_LENGTH 120

class OpenedFile{
public:
    explicit OpenedFile(const String& path):path_(path){}
    virtual ~OpenedFile(){
        LOG("[OpenedFile] close %s\n", path_.c_str());
        if (path_ == "") {
            return;
        }
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
    FileDescriber(const RefCountPtr<OpenedFile>& file, FILE_ACCESS_TYPE fileAccessType): file_access_type_(fileAccessType), fd_ref_count_(1), is_pipe_(false){
        file_ = file;
    }

    FileDescriber(const RefCountPtr<OpenedFile>& file, FILE_ACCESS_TYPE fileAccessType, int fd_ref_count): file_access_type_(fileAccessType), fd_ref_count_(fd_ref_count), is_pipe_(false){
        file_ = file;
    }


    RefCountPtr<OpenedFile>& GetFile(){
        return file_;
    }

    FILE_ACCESS_TYPE GetAccessType(){
        return file_access_type_;
    }

    // TODO(waitti) ugly code, fix it
    bool DecreaseCount(){
        --fd_ref_count_;
        if(fd_ref_count_ <= 0){
            return true;
        }
        return false;
    }

    bool ISPipe() const{
        return is_pipe_;
    }

    void MarkPipe() {
        is_pipe_ = true;
    }
private:
    FILE_ACCESS_TYPE file_access_type_;
    RefCountPtr<OpenedFile> file_;
    int fd_ref_count_;
    int is_pipe_;
};

extern FileDescriber* fd_array[FILE_DESCRIBER_ARRAY_LENGTH];
extern int fd_pipe_count;

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

        fd_pipe_count = 0;
    }

    static int OpenNewFile(const char* path, int flag, int fd){
        LOG("[FD] open file %s\n", path);
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
        Erase(fd);
    }

    static RefCountPtr<OpenedFile>& GetFile(int fd){
        if (fd < 0 || fd > FILE_DESCRIBER_ARRAY_LENGTH) {
            printf("[FileDescriber] Error fd %d\n", fd);
            panic("")
        }
        if(fd_array[fd] == nullptr){
            printf("[FileDescriber] Access NULL fd %d\n", fd);
            panic("")
        }
        return fd_array[fd]->GetFile();
    }

    static void CopyFd(int source_fd, int target_fd){
        if(!Exists(source_fd)){
            printf("[FileDescriber] fd %d not exists!\n", source_fd);
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

    static int CreatePipe(int pipefd[2], int flag){
        // TODO: flag
        int read_fd = FindUnUsedFd();
        int write_fd = FindUnusedFdBiggerOrEqual(read_fd + 1);
//        auto read_path = String("/sys/pipe/pipe_") + to_string((unsigned long long)++fd_pipe_count);
//        auto write_path = String("/sys/pipe/pipe_") + to_string((unsigned long long)++fd_pipe_count);
//        fs->open(read_path.c_str(), O_CREATE | O_RDONLY);
//        fs->open(write_path.c_str(), O_CREATE | O_WRONLY);
//        fs->pipe(read_path.c_str(), write_path.c_str(), flag, 0);
//        RefCountPtr<OpenedFile> write_file(new OpenedFile(write_path));
//        RefCountPtr<OpenedFile> read_file(new OpenedFile(read_path));
//        fd_array[read_fd] = new FileDescriber(read_file, FILE_ACCESS_TYPE::READ, 2);
//        fd_array[write_fd] = new FileDescriber(write_file, FILE_ACCESS_TYPE::WRITE, 2);
        RefCountPtr<OpenedFile> file(new OpenedFile(""));
        fd_array[read_fd] = new FileDescriber(file, FILE_ACCESS_TYPE::RW, 999);
        fd_array[write_fd] = new FileDescriber(file, FILE_ACCESS_TYPE::RW, 999);
        fd_array[read_fd]->MarkPipe();
        fd_array[write_fd]->MarkPipe();
        pipefd[0] = read_fd;
        pipefd[1] = write_fd;
        FastPipe::OpenPipe(pipefd);
        return 0;
    }
private:
    static void Check(int fd){
        if (fd < 0 || fd > FILE_DESCRIBER_ARRAY_LENGTH) {
            printf("[FileDescriber] Error fd %d\n", fd);
            panic("")
        }
        if(fd_array[fd] != nullptr){
            printf("[FD] fd %d is used\n", fd);
            panic("")
        }
    }

    static bool Exists(int fd){
        return fd_array[fd] != nullptr;
    }

    static void Erase(int fd){
        if (fd == 1) return;
        if(fd_array[fd] == nullptr){
            return;
        }
        if(fd_array[fd]->DecreaseCount()){
            delete fd_array[fd];
            fd_array[fd] = nullptr;
        }
    }
};

#endif //OS_RISC_V_FILE_DESCRIBER_H