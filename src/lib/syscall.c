#include "syscall.h"
#include "register.h"
#include "stl.h"
#include "scheduler.h"
#include "external_structs.h"
#include "file_describer.h"
#include "vfs.h"
#include "times.h"

#define return(x) context->a0=x
#define get_actual_page(x) ((x>0x80000000)?x:x+ get_running_elf_page())



// 0;
#define NOP(a) 0

Context *syscall(Context *context) {
    // Check SystemCall Number
    // printf("[SYSCALL] call id=%d\n",context->a7);
    switch (context->a7) {
        // Print Register
        // @param: x: register number
        case 0: {
            printf("[DEBUG] x%d = %d\n", context->a0, *((size_t *) context + context->a0));
            break;
        }
        case 1: {
            printf("[DEBUG] satp = 0x%x\n", context->satp);
            break;
        }
        case SYS_write: {
#define debug_write(a) printf(#a " = 0x%x\n",a)
#undef debug_write
#ifdef debug_write
            printf("-> syscall: write\n");
#endif
#define debug_write NOP
            // fd：要写入文件的文件描述符。
            // buf：一个缓存区，用于存放要写入的内容。
            // count：要写入的字节数。
            // ssize_t ret = write(fd, buf, count);
            // 返回值：成功执行，返回写入的字节数。错误，则返回-1。
            int fd = (int) context->a0;

//            printf("fd %d\n", fd);
            if (file_describer_exists(fd)) {
                fd = (int) file_describer_convert(fd);
            }
//            printf("fd %d\n", fd);
            fd = fd_get_origin_fd(fd);
//            printf("fd %d\n", fd);

//            printf("[PRINT] origin buf=0x%x\n",context->a1);
            char *buf = (char *) get_actual_page(context->a1);
//            printf("----------------------\n");
//            for(int i=0;i<10;i++){
//                printf("0x%x ",buf[i]);
//            }
//            printf("\n----------------------\n");
//            printf("[PRINT] buf=0x%x\n",buf);
            int count = (int) context->a2;

//            printf("fd %d\n", fd);


            int write_bytes = vfs_write(file_describer_array[fd].data.inode, buf, count);
            return(write_bytes);
#undef debug_write
            break;
        }
        case SYS_execve: {
            execute(get_actual_page(context->a0));
            break;
        }
        case SYS_gettimeofday: {
            get_timespec(get_actual_page(context->a0));
            return(0);
            break;
        }
        case SYS_exit: {
            exit_process(context->a0);
            break;
        }
        case SYS_getppid: { lty(get_running_ppid());
            return(get_running_ppid());
            break;
        }
        case SYS_getpid: { lty(get_running_pid());
            return(get_running_pid());
            break;
        }
        case SYS_openat: {
#define debug_openat(a) printf(#a " = 0x%x\n",a)
#undef debug_openat
#ifdef debug_openat
            printf("-> syscall: openat\n");
#endif
#define debug_openat NOP
            // fd：文件所在目录的文件描述符
            // filename：要打开或创建的文件名。如为绝对路径，则忽略fd。如为相对路径，且fd是AT_FDCWD，则filename是相对于当前工作目录来说的。如为相对路径，且fd是一个文件描述符，则filename是相对于fd所指向的目录来说的。
            // flags：必须包含如下访问模式的其中一种：O_RDONLY，O_WRONLY，O_RDWR。还可以包含文件创建标志和文件状态标志。
            // mode：文件的所有权描述。详见`man 7 inode `。
            // int ret = openat(fd, filename, flags, mode);
            // 返回值：成功执行，返回新的文件描述符。失败，返回-1。
            // TODO: 暂不支持文件所有权描述
            size_t dir_fd = context->a0;
            char *filename = (char *) get_actual_page(context->a1);
            size_t flag = context->a2;

            debug_openat(dir_fd);
            debug_openat(flag);

            if (filename[0] == '.' && filename[1] == '/') {
                char *cwd = get_running_cwd();
//                printf("cwd: %s\n", cwd);
                char *new_filename = (char *) k_malloc(strlen(cwd) + 2 + strlen(filename + 2));
                sprintf(new_filename, "%s/%s", cwd, filename + 2);
                filename = new_filename + 1;
            } else if (filename[0] == '.' && strlen(filename) == 1) {
                filename = get_running_cwd();
                // 他不是个文件夹，就让他是个文件夹
                flag |= O_DIRECTORY;
            }
            if (dir_fd != AT_FDCWD) {
                int origin_fd = fd_get_origin_fd((int) dir_fd);
                if (!(file_describer_array[origin_fd].data.inode->flag & S_IFDIR)) {
                    printf("fd %d not direct a dir\n", dir_fd);
                    panic("")
                }
                char *dir_path = file_describer_array[origin_fd].path;
                char *new_filename = (char *) k_malloc(strlen(dir_path) + 2 + strlen(filename));
                sprintf(new_filename, "%s/%s", dir_path, filename);
                filename = new_filename;
            }
//            printf("filename: %s\n", filename);
            int fd = fd_search_a_empty_file_describer();
            file_describer_bind(fd, fd);

            debug_openat(fd);

            enum File_Access_Type fileAccessType;

            if (flag == O_RDONLY)
                fileAccessType = FILE_ACCESS_READ;
            else if (flag == O_WRONLY)
                fileAccessType = FILE_ACCESS_WRITE;
            else if (flag & O_RDWR)
                fileAccessType = FILE_ACCESS_WRITE | FILE_ACCESS_READ;

//            printf("flag: 0x%x\n", flag);
            // 文件夹，提前返回
            if ((flag & O_DIRECTORY)) {
                File_Describer_Data data;
                data.inode = vfs_open(filename, (int) flag, S_IFDIR);
                if (data.inode == null) {
                    printf("can't open this dir: %s\n", filename);
                    panic("")
                }
                File_Describer_Create(fd, FILE_DESCRIBER_REGULAR, fileAccessType, data, filename);
                return(fd);
                break;
            }

            debug_openat(flag);

            File_Describer_Data data;

            data.inode = vfs_open(filename, (int) flag, S_IFREG);

//            printf("-> open 0x%x\n", data.inode);

            if (data.inode == null) {
                return(-1);
                break;
            }
            File_Describer_Create(fd, FILE_DESCRIBER_REGULAR, fileAccessType, data, filename);

            return(fd);
            break;
        }
        case SYS_read: {
#define debug_read(a) printf(#a " = 0x%x\n",a)
#undef debug_read
#ifdef debug_read
            printf("-> syscall: read\n");
#endif
#define debug_read NOP
            // fd: 文件描述符，buf: 用户空间缓冲区，count：读多少
            // ssize_t ret = read(fd, buf, count)
            // ret: 返回的字节数
            size_t fd = context->a0;
            char *buf = (char *) get_actual_page(context->a1);
            size_t count = context->a2;

            debug_read(fd);
            debug_read((size_t) buf);
            debug_read(count);

            int origin_fd = fd_get_origin_fd((int) fd);
            int read_bytes = vfs_read(file_describer_array[origin_fd].data.inode, buf, (int) count);
            if (read_bytes < 0) {
                return(-1);
            } else {
                return(read_bytes);
            }
#undef debug_read
            break;
        }
        case SYS_close: {
#define debug_close(a) printf(#a " = 0x%x\n",a)
#undef debug_close
#ifdef debug_close
            printf("-> syscall: close\n");
#endif
#define debug_close NOP
            // fd：要关闭的文件描述符。
            // int ret = close(fd);
            // 返回值：成功执行，返回0。失败，返回-1。
            size_t fd = context->a0;

            debug_close(fd);

            File_Describer_Reduce((int) fd);

            // TODO: 应该解绑文件描述符

            return(0);
#undef debug_close
            break;
        }
        case SYS_getcwd: {
#define debug_getcwd(a) printf(#a " = 0x%x\n",a)
#undef debug_getcwd
#ifdef debug_getcwd
            printf("-> syscall: getcwd\n");
#endif
#define debug_getcwd NOP
            // char *buf：一块缓存区，用于保存当前工作目录的字符串。当buf设为NULL，由系统来分配缓存区。
            // size：buf缓存区的大小。
            // long ret = getcwd(buf, size);
            // 返回值：成功执行，则返回当前工作目录的字符串的指针。失败，则返回NULL。
            size_t buf = context->a0;
            size_t size = context->a1;
            char *ret;
            char *current_work_dir = get_running_cwd();
            if (buf == 0) {
                // 系统分配缓冲区
                ret = (char *) k_malloc(size);
                bind_kernel_heap((size_t) ret);
            } else {
                ret = (char *) get_actual_page(buf);
            }
            size_t len = strlen(current_work_dir);
            memcpy(ret, current_work_dir, len);
            ret[len] = '\0';
            // TODO: 如果用户使用虚拟地址此处会返回真实地址，这不合理
            return((size_t) ret);
#undef debug_getcwd
            break;
        }
        case SYS_dup: {
#define debug_dup(a) printf(#a " = 0x%x\n",a)
#undef debug_dup
#ifdef debug_dup
            printf("-> syscall: dup\n");
#endif
#define debug_dup NOP
            // fd：被复制的文件描述符。
            // int ret = dup(fd);
            // 返回值：成功执行，返回新的文件描述符。失败，返回-1。
            int fd = (int) context->a0;
            int new_fd = fd_search_a_empty_file_describer();
            file_describer_bind(new_fd, new_fd);
            File_Describer_Plus(fd);
            File_Describer_Data data = {.redirect_fd = fd};
            File_Describer_Create(new_fd, FILE_DESCRIBER_REDIRECT, FILE_ACCESS_READ, data, null);
            return (new_fd);
#undef debug_dup
            break;
        }
        case SYS_dup3: {
#define debug_dup3(a) printf(#a " = 0x%x\n",a)
#undef debug_dup3
#ifdef debug_dup3
            printf("-> syscall: dup3\n");
#endif
#define debug_dup3 NOP
            // old：被复制的文件描述符。
            // new：新的文件描述符。
            // int ret = dup3(old, new, 0);
            // 返回值：成功执行，返回新的文件描述符。失败，返回-1。
            size_t old_fd = context->a0;
            size_t new_fd = context->a1;
            // TODO: dup3暂不支持flag参数
            size_t flags = context->a2;

            if (old_fd == new_fd) {
                return (-1);
                break;
            }
            int actual_fd = fd_search_a_empty_file_describer();

            File_Describer_Data data = {.redirect_fd = (int) old_fd};
            File_Describer_Create(actual_fd, FILE_DESCRIBER_REDIRECT, FILE_ACCESS_WRITE, data, null);
            File_Describer_Plus((int) old_fd);
            // TODO: 这里返回了new_fd但是还没有建立虚拟映射关系,只建立了系统和actual_fd之间的关系
//            printf("new fd: %d\n", new_fd);
//            printf("actual fd: %d\n", actual_fd);
            file_describer_bind(new_fd, actual_fd);

            return(new_fd);
#undef debug_dup3
            break;
        }
        case SYS_chdir: {
#define debug_chdir(a) printf(#a " = 0x%x\n",a)
#undef debug_chdir
#ifdef debug_chdir
            printf("-> syscall: chdir\n");
#endif
#define debug_chdir NOP
            // path：需要切换到的目录。
            // int ret = chdir(path);
            // 返回值：成功执行，返回0。失败，返回-1。
            char *path = (char *) get_actual_page(context->a0);
            char *current_cwd = get_running_cwd();
            strcpy(current_cwd, path);
            return(0);
#undef debug_chdir
            break;
        }
        case SYS_getdents64: {
#define debug_getdents64(a) printf(#a " = 0x%x\n",a)
#undef debug_getdents64
#ifdef debug_getdents64
            printf("-> syscall: getdents64\n");
#endif
#define debug_getdents64 NOP
            // fd：所要读取目录的文件描述符。
            // buf：一个缓存区，用于保存所读取目录的信息。
            // len：buf的大小。
            // int ret = getdents64(fd, buf, len);
            // 返回值：成功执行，返回读取的字节数。当到目录结尾，则返回0。失败，则返回-1。
            size_t fd = context->a0;
            char *buf = (char *) get_actual_page(context->a1);
            size_t len = context->a2;
            if (!(file_describer_array[fd].data.inode->flag & S_IFDIR)) {
                return (-1);
                break;
            }

            Inode *inode = vfs_search(&vfs_super_node.root_inode, file_describer_array[fd].path);
            if (inode == null || inode->first_child == null) {
                return(-1);
                break;
            }

            struct linux_dirent64 *res = (struct linux_dirent64 *) buf;
            strcpy(res->d_name, inode->first_child->name);

            return(len);
#undef debug_getdents64
            break;
        }
        case SYS_mkdirat: {
#define debug_mkdirat(a) printf(#a " = 0x%x\n",a)
#undef debug_mkdirat
#ifdef debug_mkdirat
            printf("-> syscall: mkdirat\n");
#endif
#define debug_mkdirat NOP
            // dirfd：要创建的目录所在的目录的文件描述符。
            // path：要创建的目录的名称。
            //      如果path是相对路径，则它是相对于dirfd目录而言的。
            //      如果path是相对路径，且dirfd的值为AT_FDCWD，则它是相对于当前路径而言的。
            //      如果path是绝对路径，则dirfd被忽略。
            // mode：文件的所有权描述。详见`man 7 inode `。
            // int ret = mkdirat(dirfd, path, mode);
            // 返回值：成功执行，返回0。失败，返回-1。
            // TODO: mode不支持
            int dir_fd = (int) context->a0;
            char *path = (char *) get_actual_page(context->a1);
            int mode = (int) context->a2;
            if (dir_fd == AT_FDCWD) {
                char *cwd = get_running_cwd();
                char buff[strlen(path) + strlen(cwd) + 2];

//                printf("cwd: %s\n", cwd);

                if (strcmp(cwd, "/") == 0) {
                    sprintf(buff, "/%s", path);
                } else {
                    sprintf(buff, "%s/%s", cwd, path);
                }

//                printf("buff: %s\n", buff);

                Inode *ret = vfs_mkdir(buff, mode);
                if (ret == null) {
                    return(-1);
                } else {
                    return(0);
                }
            } else {
                panic("un implement in mkdir")
            }
#undef debug_mkdirat
            break;
        }
        case SYS_unlinkat: {
#define debug_unlinkat(a) printf(#a " = 0x%x\n",a)
#undef debug_unlinkat
#ifdef debug_unlinkat
            printf("-> syscall: unlinkat\n");
#endif
#define debug_unlinkat NOP
            // dirfd：要删除的链接所在的目录。
            // path：要删除的链接的名字。如果path是相对路径，则它是相对于dirfd目录而言的。如果path是相对路径，且dirfd的值为AT_FDCWD，则它是相对于当前路径而言的。如果path是绝对路径，则dirfd被忽略。
            // flags：可设置为0或AT_REMOVEDIR。
            // int ret = unlinkat(dirfd, path, flags);
            // 返回值：成功执行，返回0。失败，返回-1。
            // TODO: 没管flag
            int dir_fd = (int)context->a0;
            char* path = (char *)get_actual_page(context->a1);
            if(dir_fd == AT_FDCWD){
                char *cwd = get_running_cwd();
                char buff[strlen(path) + strlen(cwd) + 2];

//                printf("cwd: %s\n", cwd);

                if (strcmp(cwd, "/") == 0) {
                    sprintf(buff, "/%s", path);
                } else {
                    sprintf(buff, "%s/%s", cwd, path);
                }
                vfs_delete_inode(buff);
                return(0);

            }else {
                panic("un implement in unlinkat")
            }
#undef debug_unlinkat
            break;
        }
        case SYS_times: {
            struct ES_tms *tms = get_actual_page(context->a0);
            tms->tms_utime = 1;
            tms->tms_stime = 1;
            tms->tms_cutime = 1;
            tms->tms_cstime = 1;
            return(1000);
            break;
        }
        case SYS_uname: {
            struct ES_utsname *required_uname = get_actual_page(context->a0);
            memcpy(required_uname, &ES_uname, sizeof(ES_uname));
            return(0);
            break;
        }
        case SYS_wait4: {
            return(wait(get_actual_page(context->a1)));
            break;
        }
        case SYS_nanosleep: {
            TimeVal *timeVal = get_actual_page(context->a0);
            time_seconds += timeVal->sec;
            time_macro_seconds += timeVal->usec;
            return(0);
            break;
        }
        case SYS_clone: {
            clone(context->a0, context->a1, context->a2);
            break;
        }
        case SYS_sched_yield: {
            return(0);
            yield();
            break;
        }
        case SYS_mount: {
            return (0);
            break;
        }
        case SYS_umount2: {
            return (0);
            break;
        }
        default: {
            printf("[SYSCALL] Unhandled Syscall: %d\n", context->a7);
            // shutdown();
        }
    }
    context->sepc += 4;
    return context;
}