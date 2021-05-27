#ifndef OS_RISC_V_VFS_H
#define OS_RISC_V_VFS_H

#include "../driver/fatfs/ff.h"

// 文件系统相关宏
#define AT_FDCWD (-100) //相对路径
#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002 // 可读可写
//#define O_CREATE 0x200
#define O_CREATE 0x40
#define O_DIRECTORY 0x0200000


// file type
#define S_IFMT     0170000   // bit mask for the file type bit field
#define S_IFSOCK   0140000   // socket
#define S_IFLNK    0120000   // symbolic link
#define S_IFREG    0100000   // regular file
#define S_IFBLK    0060000   // block device
#define S_IFDIR    0040000   // directory
#define S_IFCHR    0020000   // character device
#define S_IFIFO    0010000   // FIFO
// mode
#define S_ISUID      04000   // set-user-ID bit (see execve(2))
#define S_ISGID      02000   // set-group-ID bit (see below)
#define S_ISVTX      01000   // sticky bit (see below)
#define S_IRWXU      00700   // owner has read, write, and execute permission
#define S_IRUSR      00400   // owner has read permission
#define S_IWUSR      00200   // owner has write permission
#define S_IXUSR      00100   // owner has execute permission
#define S_IRWXG      00070   // group has read, write, and execute permission
#define S_IRGRP      00040   // group has read permission
#define S_IWGRP      00020   // group has write permission
#define S_IXGRP      00010   // group has execute permission
#define S_IRWXO      00007   // others  (not  in group) have read, write, and
#define S_IROTH      00004   // others have read permission
#define S_IWOTH      00002   // others have write permission
#define S_IXOTH      00001   // others have execute permission

typedef union{
    FIL fat32;
    DIR fat32_dir;
} InodeData;

typedef struct Inode{
    InodeData* data;
    int flag;
    char* name;
    struct Inode* next;
    struct Inode* first_child;
}Inode;

typedef struct {
    struct Inode root_inode;
}Supernode;

void vfs_init();
Inode* vfs_search(Inode *inode, char* path);
Inode* vfs_open(char* path, int o_flag, int s_flag);
int vfs_read(Inode *inode,  char buf[], int count);
int vfs_write(Inode* inode, char buf[], int count);
void vfs_close(Inode* inode);
Inode *vfs_mkdir(char* path, int flag);
int vfs_delete_inode(char* path);

extern Supernode vfs_super_node;

#endif //OS_RISC_V_VFS_H
