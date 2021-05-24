#include "syscall.h"
#include "register.h"
#include "stl.h"
#include "scheduler.h"
#include "external_structs.h"
#include "file_describer.h"

#define return(x) context->a0=x
#define get_actual_page(x) ((x>0x80000000)?x:x+ get_running_elf_page())

// 文件系统相关宏
#define AT_FDCWD (-100) //相对路径
#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002 // 可读可写
//#define O_CREATE 0x200
#define O_CREATE 0x40
#define O_DIRECTORY 0x0200000

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
            char *buf = (char *) get_actual_page(context->a1);
            int count = (int) context->a2;
            return(fd_write_to_file(fd, buf, count));
#undef debug_write
            break;
        }
        case SYS_exit: {
            exit_process();
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
            // TODO: 暂不支持文件所有权描述
            size_t dir_fd = context->a0;
            char *filename = (char *) get_actual_page(context->a1);
            size_t flag = context->a2, flag_bak = flag;

            debug_openat(dir_fd);
            debug_openat(flag);

            if (filename[0] == '.' && filename[1] == '/') {
                filename += 2;
            }
            if (dir_fd != AT_FDCWD) {
                if (file_describer_array[dir_fd].fileDescriberType != FILE_DESCRIBER_DIR) {
                    printf("fd %d not direct a dir\n", dir_fd);
                    panic("")
                }
                int filename_len = strlen(filename);
                int dir_filename_len = strlen(file_describer_array[dir_fd].extraData.dir_name);
                // +2 means '/' && '\0'
                char *new_file_name = (char *) k_malloc(dir_filename_len + filename_len + 2);
                memcpy(new_file_name, file_describer_array[dir_fd].extraData.dir_name, dir_filename_len);
                new_file_name[dir_filename_len] = '/';
                memcpy(new_file_name + dir_filename_len + 1, filename, filename_len);
                new_file_name[dir_filename_len + filename_len] = '\0';
                filename = new_file_name;
            }

            int fd = fd_search_a_empty_file_describer();

            debug_openat(fd);

            BYTE mode = 0;
            enum File_Access_Type fileAccessType;

            if (flag == O_RDONLY)
                fileAccessType = FILE_ACCESS_READ, mode = FA_READ, flag -= O_RDONLY;
            else if (flag == O_WRONLY)
                fileAccessType = FILE_ACCESS_WRITE, mode = FA_WRITE, flag -= O_WRONLY;
            else if (flag & O_RDWR)
                fileAccessType = FILE_ACCESS_WRITE | FILE_ACCESS_READ, mode = FA_READ | FA_WRITE, flag -= O_RDWR;

            if (flag & O_CREATE)mode |= FA_CREATE_ALWAYS, flag -= O_CREATE;

            // 文件夹，提前返回
            if (flag & O_DIRECTORY) {
                mode | FA_CREATE_ALWAYS, flag -= O_DIRECTORY;
                File_Describer_Data data;
                FRESULT result = f_opendir(&data.fat32_dir, filename);
                if (result != FR_OK) {
                    printf("can't open this dir: %s\n", filename);
                    panic("")
                }
                int dir_name_len = strlen(filename);
                char *dir_name = (char *) k_malloc((dir_name_len + 1) * sizeof(char));
                strcpy(dir_name, filename);
                File_Describer_Extra_Data  extraData;
                extraData.dir_name = dir_name;
                File_Describer_Create(fd, FILE_DESCRIBER_DIR, fileAccessType, data, extraData);
                return(fd);
                break;
            }

            if (flag != 0) {
                printf("SYS_openat unsupported flag: 0x%x\n", flag_bak);
                panic("")
            }

            debug_openat(flag);
            debug_openat(mode);

            File_Describer_Data data;
            FRESULT result = f_open(&data.fat32, filename, mode);
            if (result != FR_OK) {
                printf("can't open this file: %s\n", filename);
                panic("")
            }
            File_Describer_Extra_Data extraData;
            File_Describer_Create(fd, FILE_DESCRIBER_FILE, fileAccessType, data, extraData);

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

            int origin_fd = fd_get_origin_fd((int)fd);
            assert(file_describer_array[origin_fd].fileDescriberType == FILE_DESCRIBER_FILE)
            uint32 ret;
            FRESULT result = f_read(&file_describer_array[origin_fd].data.fat32, buf, count, &ret);
            if (result != FR_OK) {
                return(-1);
            }else{
                return(ret);
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

            File_Describer_Reduce((int)fd);

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
                // TODO: 分配给进程的缓冲区在文件退出时应该释放
                ret = (char *) k_malloc(size);
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

            File_Describer_Plus(fd);
            File_Describer_Data data = {.redirect_fd = fd};
            File_Describer_Extra_Data fakeExtraData = {.dir_name = null};
            File_Describer_Create(new_fd, FILE_DESCRIBER_REDIRECT, FILE_ACCESS_READ, data, fakeExtraData);
            return (new_fd);
#undef debug_dup
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
            return(3);
            break;
        }
        case SYS_clone: {
            // specially, sepc has dealt in cloning
            // so return directly
            clone(context->a0, context->a1, context->a2);
            yield();
            return context;
        }
        case SYS_sched_yield: {
            return(0);
            yield();
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