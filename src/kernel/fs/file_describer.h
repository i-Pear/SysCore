#ifndef OS_RISC_V_FILE_DESCRIBER_H
#define OS_RISC_V_FILE_DESCRIBER_H

#include "stdbool.h"
#include "../../lib/stl/stl.h"
#include "VFS.h"
#include "../../lib/stl/RefCountPtr.h"
#include "../../lib/stl/string.h"
#include "../../lib/stl/map.h"

#define FILE_DESCRIBER_ARRAY_LENGTH 120
#define FILE_DESCRIBER_RESERVED_FD_COUNT 3

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

//enum File_Describer_Type {
//    FILE_DESCRIBER_REGULAR,
//    FILE_DESCRIBER_REDIRECT
//};
//
//enum File_Access_Type {
//    FILE_ACCESS_READ = 1,
//    FILE_ACCESS_WRITE = 2,
//    FILE_ACCESS_RW = 3
//};
//
//typedef union {
//    int redirect_fd;
//} File_Describer_Data;
//
//typedef struct {
//    enum File_Describer_Type fileDescriberType;
//    enum File_Access_Type fileAccessType;
//    File_Describer_Data data;
//    char *path;
//} File_Describer;

//// 最大文件描述符个数
//#define FILE_DESCRIBER_ARRAY_LENGTH 120
//
//extern File_Describer file_describer_array[];
//extern int file_describer_array_occupied[];
//
///**
// * 初始化VFS
// */
//void init_file_describer();
//
///**
// * 在fd处初始化一个文件描述符
// * @param fd
// * @param fileDescriberType：文件描述符类型
// * @param fileAccessType：文件访问权限
// * @param data：数据
// * @param extraData：额外数据
// */
//void File_Describer_Create(
//        int fd,
//        enum File_Describer_Type fileDescriberType,
//        enum File_Access_Type fileAccessType,
//        File_Describer_Data data,
//        char *path);
//
///**
// * fd引用计数加一
// * @param fd
// */
//void File_Describer_Plus(int fd);
//
///**
// * fd引用计数减一
// * 如果fd是重定向文件描述符，那么包括此fd在内一直到源文件描述符的引用计数都会减一
// * 如果引用计数为0则释放该文件描述符
// * @param fd
// */
//void File_Describer_Reduce(int fd);
//
//char *File_Describer_Get_Path(int fd);
//
///*
// *  私有接口
// */
//
//int fd_write_to_file(int fd, char *buf, int count);
//
//int fd_get_origin_fd(int fd);
//
//int fd_search_a_empty_file_describer();
//
//int fd_search_a_empty_file_describer_bigger_or_equal_than_arg(int arg);

#endif //OS_RISC_V_FILE_DESCRIBER_H