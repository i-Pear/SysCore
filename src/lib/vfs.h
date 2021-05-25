#ifndef OS_RISC_V_VFS_H
#define OS_RISC_V_VFS_H

// 文件系统相关宏
#define AT_FDCWD (-100) //相对路径
#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002 // 可读可写
//#define O_CREATE 0x200
#define O_CREATE 0x40
#define O_DIRECTORY 0x0200000

enum Inode_Type{
    INODE_TYPE_FILE,
    INODE_TYPE_DIR,
    INODE_TYPE_STDOUT
};

typedef struct Inode{
    enum Inode_Type inode_type;
    char* name;
    struct Inode* next;
    struct Inode* first_child;
}Inode;

typedef struct {
    struct Inode root_inode;
}Supernode;

void vfs_init();
Inode* vfs_search(Inode *inode, char* path);

extern Supernode vfs_super_node;

#endif //OS_RISC_V_VFS_H
