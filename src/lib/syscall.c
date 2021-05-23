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
            int file = context->a0;
            char *buf = get_actual_page(context->a1);
            int count = context->a2;
            if (file == 1) {
                // stdout
                for (int i = 0; i < count; i++)putchar(buf[i]);
                return(count);
            } else if (file == 2) {
                // stderr
                for (int i = 0; i < count; i++)putchar(buf[i]);
                return(count);
            } else {
                // Other files
            }
            break;
        }
        case SYS_exit: {
            exit_process();
            break;
        }
        case SYS_getppid:{
            lty(get_running_ppid());
            return(get_running_ppid());
            break;
        }
        case SYS_getpid: {
            lty(get_running_pid());
            return(get_running_pid());
            break;
        }
        case SYS_openat:{
#define debug_openat(a) printf(#a " = 0x%x\n",a)
#undef debug_openat
#define debug_openat ;
            // fd：文件所在目录的文件描述符
            // filename：要打开或创建的文件名。如为绝对路径，则忽略fd。如为相对路径，且fd是AT_FDCWD，则filename是相对于当前工作目录来说的。如为相对路径，且fd是一个文件描述符，则filename是相对于fd所指向的目录来说的。
            // flags：必须包含如下访问模式的其中一种：O_RDONLY，O_WRONLY，O_RDWR。还可以包含文件创建标志和文件状态标志。
            // mode：文件的所有权描述。详见`man 7 inode `。
            // int ret = openat(fd, filename, flags, mode);
            // TODO: 暂不支持通过fd打开
            // TODO: 暂不支持文件所有权描述
            size_t dir_fd = context->a0;
            if(dir_fd != AT_FDCWD){
                panic("SYS_openat 不支持通过fd打开");
            }
            char* filename = (char*)get_actual_page(context->a1);
            size_t flag = context->a2;

            debug_openat(dir_fd);
            debug_openat(flag);

            if(filename[0] == '.' && filename[1] == '/'){
                filename += 2;
            }
            int fd = get_new_file_describer();

            debug_openat(fd);

            BYTE mode = 0;
            if(flag == O_RDONLY)file_describer_array[fd].fileAccessType = FILE_ACCESS_READ, mode = FA_READ;
            else if(flag == O_WRONLY)file_describer_array[fd].fileAccessType = FILE_ACCESS_WRITE, mode = FA_WRITE;
            else if(flag == O_RDWR)file_describer_array[fd].fileAccessType = FILE_ACCESS_WRITE | FILE_ACCESS_READ, mode = FA_READ | FA_WRITE;
            else{
                printf("SYS_openat 不支持的flag: 0x%x\n", flag);
                panic("")
            }

            debug_openat(mode);

            file_describer_array[fd].fileDescriberType = FILE_DESCRIBER_FILE;
            FRESULT result = f_open(&file_describer_array[fd].data.fat32, filename, mode);
            if(result != FR_OK){
                printf("can't open this file: %s\n", filename);
                panic("")
            }
            return(fd);
#undef debug_openat
            break;
        }
        case SYS_read:{
#define debug_read(a) printf(#a " = 0x%x\n",a)
#undef debug_read
#define debug_read ;
            // fd: 文件描述符，buf: 用户空间缓冲区，count：读多少
            // ssize_t ret = read(fd, buf, count)
            // ret: 返回的字节数
            size_t fd = context->a0;
            char* buf = (char*)get_actual_page(context->a1);
            size_t count = context->a2;

            debug_read(fd);
            debug_read((size_t)buf);
            debug_read(count);

            FIL fat_file = file_describer_array[fd].data.fat32;
            uint32 ret;
            FRESULT result = f_read(&fat_file, buf, count, &ret);
            if(result != FR_OK){
                printf("can't read this file: fd = %d\n", fd);
                panic("")
            }
            return(ret);
#undef debug_read
            break;
        }
        case SYS_close:{
#define debug_close(a) printf(#a " = 0x%x\n",a)
#undef debug_close
#define debug_close ;
            // fd：要关闭的文件描述符。
            // int ret = close(fd);
            // 返回值：成功执行，返回0。失败，返回-1。
            size_t fd = context->a0;

            debug_close(fd);

            FRESULT result = f_close(&file_describer_array[fd].data.fat32);
            if(result != FR_OK){
                return(-1);
            }else{
                erase_file_describer((int)fd);
                return(0);
            }
#undef debug_close
            break;
        }
        case SYS_times:{
            struct ES_tms* tms=get_actual_page(context->a0);
            tms->tms_utime=1;
            tms->tms_stime=1;
            tms->tms_cutime=1;
            tms->tms_cstime=1;
            return(1000);
            break;
        }
        case SYS_uname: {
            struct ES_utsname *required_uname = get_actual_page(context->a0);
            memcpy(required_uname, &ES_uname, sizeof(ES_uname));
            return(0);
            break;
        }
        case SYS_wait4:{

            break;
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