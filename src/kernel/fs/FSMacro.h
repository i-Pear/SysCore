#ifndef OS_RISC_V_FSMACRO_H
#define OS_RISC_V_FSMACRO_H

/// open mode

#define AT_FDCWD (-100) // relative path
#define O_RDONLY 0x000 // read only
#define O_WRONLY 0x001  // write only
#define O_RDWR 0x002 // read & write
#define O_CREATE 0x40  // always create
#define O_DIRECTORY 0x0200000 // it's a directory

/// file type

#define S_IFMT     0170000   // bit mask for the file type bit field
#define S_IFSOCK   0140000   // socket
#define S_IFLNK    0120000   // symbolic link
#define S_IFREG    0100000   // regular file
#define S_IFBLK    0060000   // block device
#define S_IFDIR    0040000   // directory
#define S_IFCHR    0020000   // character device
#define S_IFIFO    0010000   // FIFO

/// file mode

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

/// dirent mode


#define DT_UNKNOWN 0         //未知类型
#define DT_FIFO 1           //管道
#define DT_CHR  2             //字符设备
#define DT_DIR    4             //目录
#define DT_BLK    6            //块设备
#define DT_REG    8            //常规文件
#define DT_LNK    10           //符号链接
#define DT_SOCK    12          //套接字
#define DT_WHT    14             //链接

/// lseek whence
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2


#endif //OS_RISC_V_FSMACRO_H
