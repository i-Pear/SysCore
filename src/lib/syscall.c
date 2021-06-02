#include "syscall.h"
#include "register.h"
#include "stl.h"
#include "scheduler.h"
#include "external_structs.h"
#include "file_describer.h"
#include "vfs.h"
#include "times.h"

#define get_actual_page(x) ((x>0x80000000)?x:x+ get_running_elf_page())
#define SYSCALL_LIST_LENGTH (1024)

// lazy init syscall list
int (*syscall_list[SYSCALL_LIST_LENGTH])(Context *context);

int syscall_is_initialized = 0;

void syscall_register();
void syscall_distribute(int syscall_id, Context *context);


// 0;
#define NOP(a) 0

int sysGetRealFd(int fd) {
    if (file_describer_exists(fd)) {
        fd = (int) file_describer_convert(fd);
    }
    fd = fd_get_origin_fd(fd);
    return fd;
}

int sys_getchar(Context *context) {
    return getchar_blocked();
}

int sys_write(Context *context) {
    // fd：要写入文件的文件描述符。
    // buf：一个缓存区，用于存放要写入的内容。
    // count：要写入的字节数。
    // ssize_t ret = write(fd, buf, count);
    // 返回值：成功执行，返回写入的字节数。错误，则返回-1。
    int fd = sysGetRealFd((int) context->a0);
    char *buf = (char *) get_actual_page(context->a1);
    int count = (int) context->a2;
    int write_bytes = vfs_write(file_describer_array[fd].data.inode, buf, count);
    return write_bytes;
}

int sys_execve(Context *context) {
    execute(get_actual_page(context->a0));
    return context->a0;
}

int sys_gettimeofday(Context *context) {
    get_timespec(get_actual_page(context->a0));
    return 0;
}

int sys_exit(Context *context) {
    exit_process(context->a0);
    return context->a0;
}

int sys_getppid(Context *context) {
    return get_running_ppid();
}

int sys_getpid(Context *context) {
    return get_running_pid();
}

int sys_openat(Context *context) {
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

    if (filename[0] == '.' && filename[1] == '/') {
        char *cwd = get_running_cwd();
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
    int fd = fd_search_a_empty_file_describer();
    file_describer_bind(fd, fd);

    enum File_Access_Type fileAccessType;

    if (flag == O_RDONLY)
        fileAccessType = FILE_ACCESS_READ;
    else if (flag == O_WRONLY)
        fileAccessType = FILE_ACCESS_WRITE;
    else if (flag & O_RDWR)
        fileAccessType = FILE_ACCESS_WRITE | FILE_ACCESS_READ;

    // 文件夹，提前返回
    if ((flag & O_DIRECTORY)) {
        File_Describer_Data data;
        data.inode = vfs_open(filename, (int) flag, S_IFDIR);
        if (data.inode == null) {
            printf("can't open this dir: %s\n", filename);
            panic("")
        }
        File_Describer_Create(fd, FILE_DESCRIBER_REGULAR, fileAccessType, data, filename);
        return (fd);
    }

    File_Describer_Data data;

    data.inode = vfs_open(filename, (int) flag, S_IFREG);

    if (data.inode == null) {
        return (-1);
    }
    File_Describer_Create(fd, FILE_DESCRIBER_REGULAR, fileAccessType, data, filename);

    return (fd);
}

int sys_read(Context *context) {
    // fd: 文件描述符，buf: 用户空间缓冲区，count：读多少
    // ssize_t ret = read(fd, buf, count)
    // ret: 返回的字节数
    int fd = sysGetRealFd(context->a0);
    char *buf = (char *) get_actual_page(context->a1);
    size_t count = context->a2;

    int read_bytes = vfs_read(file_describer_array[fd].data.inode, buf, (int) count);
    if (read_bytes < 0) {
        return (-1);
    } else {
        return (read_bytes);
    }
}

int sys_close(Context *context) {
    // fd：要关闭的文件描述符。
    // int ret = close(fd);
    // 返回值：成功执行，返回0。失败，返回-1。
    size_t fd = sysGetRealFd(context->a0);

    File_Describer_Reduce((int) fd);

    // TODO: 应该解绑文件描述符

    return (0);
}

int sys_getcwd(Context *context) {
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
    return (int) ((size_t) ret);
}

int sys_dup(Context *context) {
    // fd：被复制的文件描述符。
    // int ret = dup(fd);
    // 返回值：成功执行，返回新的文件描述符。失败，返回-1。
    int fd = sysGetRealFd(context->a0);
    int new_fd = fd_search_a_empty_file_describer();
    file_describer_bind(new_fd, new_fd);
    File_Describer_Plus(fd);
    File_Describer_Data data = {.redirect_fd = fd};
    File_Describer_Create(new_fd, FILE_DESCRIBER_REDIRECT, FILE_ACCESS_READ, data, null);
    return (new_fd);
}

int sys_dup3(Context *context) {
    // old：被复制的文件描述符。
    // new：新的文件描述符。
    // int ret = dup3(old, new, 0);
    // 返回值：成功执行，返回新的文件描述符。失败，返回-1。
    size_t old_fd = sysGetRealFd(context->a0);
    size_t new_fd = context->a1;
    // TODO: dup3暂不支持flag参数
    size_t flags = context->a2;

    if (old_fd == new_fd) {
        return (-1);
    }
    int actual_fd = fd_search_a_empty_file_describer();

    File_Describer_Data data = {.redirect_fd = (int) old_fd};
    File_Describer_Create(actual_fd, FILE_DESCRIBER_REDIRECT, FILE_ACCESS_WRITE, data, null);
    File_Describer_Plus((int) old_fd);
    // TODO: 这里返回了new_fd但是还没有建立虚拟映射关系,只建立了系统和actual_fd之间的关系
//            printf("new fd: %d\n", new_fd);
//            printf("actual fd: %d\n", actual_fd);
    file_describer_bind(new_fd, actual_fd);

    return (int) (new_fd);
}

int sys_chdir(Context *context) {
    // path：需要切换到的目录。
    // int ret = chdir(path);
    // 返回值：成功执行，返回0。失败，返回-1。
    char *path = (char *) get_actual_page(context->a0);
    char *current_cwd = get_running_cwd();
    strcpy(current_cwd, path);
    return (0);
}

int sys_getdents64(Context *context) {
    // fd：所要读取目录的文件描述符。
    // buf：一个缓存区，用于保存所读取目录的信息。
    // len：buf的大小。
    // int ret = getdents64(fd, buf, len);
    // 返回值：成功执行，返回读取的字节数。当到目录结尾，则返回0。失败，则返回-1。
    size_t fd = sysGetRealFd(context->a0);
    char *buf = (char *) get_actual_page(context->a1);
    size_t len = context->a2;
    if (!(file_describer_array[fd].data.inode->flag & S_IFDIR)) {
        return (-1);
    }

    Inode *inode = vfs_search(&vfs_super_node.root_inode, file_describer_array[fd].path);
    if (inode == null || inode->first_child == null) {
        return (-1);
    }

    struct linux_dirent64 *res = (struct linux_dirent64 *) buf;
    strcpy(res->d_name, inode->first_child->name);

    return (len);
}

int sys_mkdirat(Context *context) {
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
            return (-1);
        } else {
            return (0);
        }
    } else {
        panic("un implement in mkdir")
        return -1;
    }
}

int sys_unlinkat(Context *context) {
    // dirfd：要删除的链接所在的目录。
    // path：要删除的链接的名字。如果path是相对路径，则它是相对于dirfd目录而言的。如果path是相对路径，且dirfd的值为AT_FDCWD，则它是相对于当前路径而言的。如果path是绝对路径，则dirfd被忽略。
    // flags：可设置为0或AT_REMOVEDIR。
    // int ret = unlinkat(dirfd, path, flags);
    // 返回值：成功执行，返回0。失败，返回-1。
    // TODO: 没管flag
    int dir_fd = (int) context->a0;
    char *path = (char *) get_actual_page(context->a1);
    if (dir_fd == AT_FDCWD) {
        char *cwd = get_running_cwd();
        char buff[strlen(path) + strlen(cwd) + 2];

//                printf("cwd: %s\n", cwd);

        if (strcmp(cwd, "/") == 0) {
            sprintf(buff, "/%s", path);
        } else {
            sprintf(buff, "%s/%s", cwd, path);
        }
        vfs_delete_inode(buff);
        return (0);

    } else {
        panic("un implement in unlinkat")
        return -1;
    }
}

int sys_times(Context* context){
    struct ES_tms *tms = get_actual_page(context->a0);
    tms->tms_utime = 1;
    tms->tms_stime = 1;
    tms->tms_cutime = 1;
    tms->tms_cstime = 1;
    return (1000);
}

int sys_uname(Context* context){
    struct ES_utsname *required_uname = get_actual_page(context->a0);
    memcpy(required_uname, &ES_uname, sizeof(ES_uname));
    return (0);
}

int sys_wait4(Context* context){
    return (wait(get_actual_page(context->a1)));
}

int sys_nanosleep(Context* context){
    TimeVal *timeVal = get_actual_page(context->a0);
    time_seconds += timeVal->sec;
    time_macro_seconds += timeVal->usec;
    return (0);
}

int sys_clone(Context* context){
    clone(context->a0, context->a1, context->a2);
    return context->a0;
}

int sys_sched_yield(Context* context){
    yield();
    return (0);
}

int sys_mount(Context* context){
    return (0);
}

int sys_umount2(Context* context){
    return 0;
}

void syscall_init() {
    for (int i = 0; i < SYSCALL_LIST_LENGTH; i++) {
        syscall_list[i] = NULL;
    }
}

void syscall_unhandled(Context *context) {
    printf("[SYSCALL] Unhandled Syscall: %d\n", context->a7);
}

void syscall_distribute(int syscall_id, Context *context) {
    if (syscall_is_initialized != 1) {
        syscall_init();
        syscall_register();
        syscall_is_initialized = 1;
    }

    assert(syscall_id >= 0 && syscall_id < SYSCALL_LIST_LENGTH);
    assert(context != NULL);

    if (syscall_list[syscall_id] != NULL) {
        context->a0 = syscall_list[syscall_id](context);
    } else {
        syscall_unhandled(context);
    }
}

Context *syscall(Context *context) {
    syscall_distribute((int) context->a7, context);
    context->sepc += 4;
    return context;
}


void syscall_register() {
    syscall_list[SYS_getchar] = sys_getchar;
    syscall_list[SYS_write] = sys_write;
    syscall_list[SYS_execve] = sys_execve;
    syscall_list[SYS_gettimeofday] = sys_gettimeofday;
    syscall_list[SYS_exit] = sys_exit;
    syscall_list[SYS_getppid] = sys_getppid;
    syscall_list[SYS_getpid] = sys_getpid;
    syscall_list[SYS_openat] = sys_openat;
    syscall_list[SYS_read] = sys_read;
    syscall_list[SYS_close] = sys_close;
    syscall_list[SYS_getcwd] = sys_getcwd;
    syscall_list[SYS_dup] = sys_dup;
    syscall_list[SYS_dup3] = sys_dup3;
    syscall_list[SYS_chdir] = sys_chdir;
    syscall_list[SYS_getdents64] = sys_getdents64;
    syscall_list[SYS_mkdirat] = sys_mkdirat;
    syscall_list[SYS_unlinkat] = sys_unlinkat;
    syscall_list[SYS_times] = sys_times;
    syscall_list[SYS_uname] = sys_uname;
    syscall_list[SYS_wait4] = sys_wait4;
    syscall_list[SYS_nanosleep] = sys_nanosleep;
    syscall_list[SYS_clone] = sys_clone;
    syscall_list[SYS_sched_yield] = sys_sched_yield;
    syscall_list[SYS_mount] = sys_mount;
    syscall_list[SYS_umount2] = sys_umount2;
}

