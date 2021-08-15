#include "syscall.h"
#include "register.h"
#include "../lib/stl/stl.h"
#include "scheduler.h"
#include "time/time.h"
#include "posix/posix_structs.h"
#include "posix/fcntl.h"
#include "fs/file_describer.h"
#include "memory/Heap.h"
#include "posix/errno.h"
#include "posix/pselect.h"

//#define get_actual_page(x) (((x)>0x80000000)?(x):(x)+ get_running_elf_page())
#define get_actual_page(x) (x)
#define SYSCALL_LIST_LENGTH (1024)
// 该宏默认所有的路径长度都不会超过512
#define PATH_BUFF_SIZE (512)

/// lazy init syscall list
size_t (*syscall_list[SYSCALL_LIST_LENGTH])(Context *context);
/// syscall id -> name map
char* syscall_name_list[SYSCALL_LIST_LENGTH];


/// functions declaration
void syscall_register();

void syscall_distribute(int syscall_id, Context *context);


/// utils functions

int sysGetRealFd(int fd) {
    if (file_describer_exists(fd)) {
        fd = (int) file_describer_convert(fd);
    }
    return fd;
}

/**
 * strpbrk - Find the first occurrence of a set of characters
 * @cs: The string to be searched
 * @ct: The characters to search for
 */
char *strpbrk(const char *cs, const char *ct) {
    const char *sc1, *sc2;

    for (sc1 = cs; *sc1 != '\0'; ++sc1) {
        for (sc2 = ct; *sc2 != '\0'; ++sc2) {
            if (*sc1 == *sc2)
                return (char *) sc1;
        }
    }
    return NULL;
}

/**
 * strsep - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 *
 * strsep() updates @s to point after the token, ready for the next call.
 *
 * It returns empty tokens, too, behaving exactly like the libc function
 * of that name. In fact, it was stolen from glibc2 and de-fancy-fied.
 * Same semantics, slimmer shape. ;)
 */
char *strsep(char **s, const char *ct) {
    char *sbegin = *s;
    char *end;

    if (sbegin == NULL)
        return NULL;

    end = strpbrk(sbegin, ct);
    if (end)
        *end++ = '\0';
    *s = end;
    return sbegin;
}

void test_getAbsolutePath();

/**
 * return absolute path by path & cwd
 * @param path
 * @param cwd
 * @return if valid, return a temp address; otherwise, return NULL.
 */
char *getAbsolutePath(char *path, char *cwd) {
#define PATH_MAX_LEN 512
#define PATH_UNIT_MAX_COUNT 16
    static int getAbsolutePathIsInitialized = 0;
    if (getAbsolutePathIsInitialized == 0) {
        getAbsolutePathIsInitialized = 1;
        test_getAbsolutePath();
    }
    static char str_buff[PATH_MAX_LEN];
    static char units[PATH_UNIT_MAX_COUNT][PATH_MAX_LEN];
    static char res[PATH_UNIT_MAX_COUNT][PATH_MAX_LEN];
    memset(str_buff, 0, sizeof str_buff);
    if (cwd != NULL) {
        // if path start with /, it means absolute path
        if (!(strlen(path) >= 1 && path[0] == '/')) {
            strcpy(str_buff, cwd);
            // if cwd don't end with /, just add /
            if (strlen(cwd) == 0 || cwd[strlen(cwd) - 1] != '/') {
                strcpy(str_buff + strlen(str_buff), "/");
            }
        }
    }

    if (path == NULL) {
        if (cwd == NULL)return NULL;
    } else {
        strcpy(str_buff + strlen(str_buff), path);
    }

    int cur_unit = 0;
    char *buf_pointer = str_buff;
    char *unit;
    while ((unit = strsep(&buf_pointer, "/")) != NULL) {
        if (*unit != '\0') {
            strcpy(units[cur_unit++], unit);
        }
    }

    int unit_count = cur_unit;
    cur_unit = 0;

    int cur = 0;
    while (cur_unit < unit_count) {
        if (strcmp(units[cur_unit], ".") == 0) {
            // do nothing
        } else if (strcmp(units[cur_unit], "..") == 0) {
            cur--;
            if (cur < 0) {
                return NULL;
            }
        } else {
            strcpy(res[cur], units[cur_unit]);
            cur++;
            if (cur >= PATH_UNIT_MAX_COUNT) {
                return NULL;
            }
        }
        cur_unit++;
    }

    for (int i = 0; i < cur; i++) {
        strcpy(str_buff + strlen(str_buff), "/");
        strcpy(str_buff + strlen(str_buff), res[i]);
    }

    // if root, should return /
    if (cur == 0) {
        strcpy(str_buff, "/");
    }
    return str_buff;
#undef PATH_MAX_LEN
#undef PATH_UNIT_MAX_COUNT
}

void test_getAbsolutePath() {
    // positive test
    assert(strcmp(getAbsolutePath(".//mnt/test/../", "/"), "/mnt") == 0);
    assert(strcmp(getAbsolutePath("/", NULL), "/") == 0);
    assert(strcmp(getAbsolutePath(".", NULL), "/") == 0);
    assert(strcmp(getAbsolutePath("../", "/mnt/"), "/") == 0);
    assert(strcmp(getAbsolutePath("../", "/mnt"), "/") == 0);
    assert(strcmp(getAbsolutePath(".", "/"), "/") == 0);
    assert(strcmp(getAbsolutePath("/test", NULL), "/test") == 0);
    assert(strcmp(getAbsolutePath("./test", "/"), "/test") == 0);
    assert(strcmp(getAbsolutePath("././", NULL), "/") == 0);
    assert(strcmp(getAbsolutePath(".", "/mnt/"), "/mnt") == 0);
    assert(strcmp(getAbsolutePath("./", "/mnt/"), "/mnt") == 0);
    assert(strcmp(getAbsolutePath(".", "/mnt"), "/mnt") == 0);
    assert(strcmp(getAbsolutePath("./", "/mnt"), "/mnt") == 0);
    assert(strcmp(getAbsolutePath("/mnt/test_mount", NULL), "/mnt/test_mount") == 0);
    assert(strcmp(getAbsolutePath("/mnt/test_mount", "/"), "/mnt/test_mount") == 0);
//    assert(strcmp(getAbsolutePath(NULL, "/"), "/") == 0);

    // negative test
    assert(getAbsolutePath("../../", "/") == NULL);
//    assert(getAbsolutePath(NULL, NULL) == NULL);
}

char *atFdCWD(int dir_fd, char *path) {
    static char buff[PATH_BUFF_SIZE];
    if (dir_fd == AT_FDCWD) {
        if(path != NULL){
            strcpy(buff, getAbsolutePath(path, get_running_cwd()));
        }else{
            strcpy(buff, get_running_cwd());
        }
    } else {
        if(path != NULL){
            strcpy(buff, getAbsolutePath(path, FD::GetFile(dir_fd)->GetCStylePath()));
        }else{
            strcpy(buff, FD::GetFile(dir_fd)->GetCStylePath());
        }
    }
    return buff;
}

/// syscall

size_t sys_getchar(Context *context) {
    return getchar_blocked();
}

size_t sys_write(Context *context) {
    // fd：要写入文件的文件描述符。
    // buf：一个缓存区，用于存放要写入的内容。
    // count：要写入的字节数。
    // ssize_t ret = write(fd, buf, count);
    // 返回值：成功执行，返回写入的字节数。错误，则返回-1。
    int fd = sysGetRealFd((int) context->a0);
    char *buf = (char *) get_actual_page(context->a1);
    int count = (int) context->a2;
    int write_bytes = fs->write(FD::GetFile(fd)->GetCStylePath(), buf, count);
    return write_bytes;
}

size_t sys_execve(Context *context) {
    execute((const char *)get_actual_page(context->a0));
    return context->a0;
}

size_t sys_gettimeofday(Context *context) {
    auto timeVal=(TimeVal*)context->a0;
    uint64 usec=timer();
    timeVal->sec=usec/1000000;
    timeVal->usec=usec%1000000;

    return 0;
}

size_t sys_exit(Context *context) {
    printf("Program exited with exit code 0x%x\n",context->a0);
    exit_process(context->a0);
    return context->a0;
}

size_t sys_getppid(Context *context) {
    return get_running_ppid();
}

size_t sys_getpid(Context *context) {
    return get_running_pid();
}

size_t sys_openat(Context *context) {
    // fd：文件所在目录的文件描述符
    // filename：要打开或创建的文件名。如为绝对路径，则忽略fd。如为相对路径，且fd是AT_FDCWD，则filename是相对于当前工作目录来说的。如为相对路径，且fd是一个文件描述符，则filename是相对于fd所指向的目录来说的。
    // flags：必须包含如下访问模式的其中一种：O_RDONLY，O_WRONLY，O_RDWR。还可以包含文件创建标志和文件状态标志。
    // mode：文件的所有权描述。详见`man 7 inode `。
    // int ret = openat(fd, filename, flags, mode);
    // 返回值：成功执行，返回新的文件描述符。失败，返回-1。
    // TODO: 暂不支持文件所有权描述
    int dir_fd = sysGetRealFd(context->a0);
    char *filename = (char *) get_actual_page(context->a1);
    int flag = context->a2;
    int fd = FD::FindUnUsedFd();
    char* path = atFdCWD(dir_fd, filename);
    int fr = FD::OpenNewFile(path, flag, fd);
    if(fr != 0){
        return -1;
    }
    return fd;
}

size_t sys_read(Context *context) {
    // fd: 文件描述符，buf: 用户空间缓冲区，count：读多少
    // ssize_t ret = read(fd, buf, count)
    // ret: 返回的字节数
    int fd = sysGetRealFd(context->a0);
    char *buf = (char *) get_actual_page(context->a1);
    int count = (int)context->a2;
    return fs->read(FD::GetFile(fd)->GetCStylePath(), buf, count);
}

size_t sys_close(Context *context) {
    // fd：要关闭的文件描述符。
    // int ret = close(fd);
    // 返回值：成功执行，返回0。失败，返回-1。
    int fd = sysGetRealFd(context->a0);
    FD::TryCloseFile(fd);
    return (0);
}

size_t sys_getcwd(Context *context) {
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
    strcpy(ret, current_work_dir);
    // TODO: 如果用户使用虚拟地址此处会返回真实地址，这不合理
    return strlen(ret)+1;
}

size_t sys_dup(Context *context) {
    // fd：被复制的文件描述符。
    // int ret = dup(fd);
    // 返回值：成功执行，返回新的文件描述符。失败，返回-1。
    int fd = sysGetRealFd(context->a0);
    int new_fd = FD::FindUnUsedFd();
    file_describer_bind(new_fd, new_fd);
    FD::CopyFd(fd, new_fd);
    return (new_fd);
}

size_t sys_dup3(Context *context) {
    // old：被复制的文件描述符。
    // new：新的文件描述符。
    // int ret = dup3(old, new, 0);
    // 返回值：成功执行，返回新的文件描述符。失败，返回-1。
    size_t old_fd = sysGetRealFd(context->a0);
    size_t new_fd = sysGetRealFd(context->a1);
    // TODO: dup3暂不支持flag参数
    size_t flags = context->a2;

    if (old_fd == new_fd) {
        return (-1);
    }

    FD::CopyFd(old_fd, new_fd);

    return (int) (new_fd);
}

size_t sys_chdir(Context *context) {
    // path：需要切换到的目录。
    // int ret = chdir(path);
    // 返回值：成功执行，返回0。失败，返回-1。
    char *path = (char *) get_actual_page(context->a0);
    char *current_cwd = get_running_cwd();
    if(current_cwd != NULL){
        strcpy(current_cwd, getAbsolutePath(path, get_running_cwd()));
    }else{
        strcpy(current_cwd, get_running_cwd());
    }
    return (0);
}

size_t sys_getdents64(Context *context) {
    // fd：所要读取目录的文件描述符。
    // buf：一个缓存区，用于保存所读取目录的信息。
    // len：buf的大小。
    // int ret = getdents64(fd, buf, len);
    // 返回值：成功执行，返回读取的字节数。当到目录结尾，则返回0。失败，则返回-1。
    size_t fd = sysGetRealFd(context->a0);
    char *buf = (char *) get_actual_page(context->a1);
    size_t len = context->a2;
    return fs->read_dir(FD::GetFile(fd)->GetCStylePath(), buf, len);
}

size_t sys_mkdirat(Context *context) {
    // dirfd：要创建的目录所在的目录的文件描述符。
    // path：要创建的目录的名称。
    //      如果path是相对路径，则它是相对于dirfd目录而言的。
    //      如果path是相对路径，且dirfd的值为AT_FDCWD，则它是相对于当前路径而言的。
    //      如果path是绝对路径，则dirfd被忽略。
    // mode：文件的所有权描述。详见`man 7 inode `。
    // int ret = mkdirat(dirfd, path, mode);
    // 返回值：成功执行，返回0。失败，返回-1。
    // TODO: mode不支持
    int dir_fd = sysGetRealFd(context->a0);
    char *path = (char *) get_actual_page(context->a1);
    int mode = (int) context->a2;
    fs->mkdir(atFdCWD(dir_fd, path), mode);
    return 0;
}

size_t sys_unlinkat(Context *context) {
    // dirfd：要删除的链接所在的目录。
    // path：要删除的链接的名字。如果path是相对路径，则它是相对于dirfd目录而言的。如果path是相对路径，且dirfd的值为AT_FDCWD，则它是相对于当前路径而言的。如果path是绝对路径，则dirfd被忽略。
    // flags：可设置为0或AT_REMOVEDIR。
    // int ret = unlinkat(dirfd, path, flags);
    // 返回值：成功执行，返回0。失败，返回-1。
    // TODO: 没管flag
    int dir_fd = sysGetRealFd(context->a0);
    char *path = (char *) get_actual_page(context->a1);
    // TODO: we can't really delete it
    fs->unlink(atFdCWD(dir_fd, path));
    return 0;
}

size_t sys_times(Context *context) {
    struct ES_tms *tms = (struct ES_tms *)get_actual_page(context->a0);
    tms->tms_utime = 1;
    tms->tms_stime = 1;
    tms->tms_cutime = 1;
    tms->tms_cstime = 1;
    return (1000);
}

size_t sys_uname(Context *context) {
    struct ES_utsname *required_uname = (struct ES_utsname *)get_actual_page(context->a0);
    memcpy(required_uname, &ES_uname, sizeof(ES_uname));
    return (0);
}

size_t sys_wait4(Context *context) {
    return (wait((int *)get_actual_page(context->a1)));
}

size_t sys_clock_nanosleep(Context *context) {
    int clock_id=context->a0;
    int flags=context->a1;
    const auto *request= (const timespec*)(context->a2);
    auto *remain = (timespec *)(context->a3);

    remain->tv_sec=0;
    remain->tv_nsec=0;

    return 0;
}

size_t sys_clone(Context *context) {
    clone(context->a0, context->a1, (int*)context->a2, context->a3,(int*)context->a4);
    return context->a0;
}

size_t sys_sched_yield(Context *context) {
    yield();
    return (0);
}

size_t sys_mprotect(Context* context){
    return 0;
}

size_t sys_mount(Context *context) {
    return (0);
}

size_t sys_umount2(Context *context) {
    return 0;
}

size_t sys_fstat(Context* context){
    return 0;

    // fd: 文件句柄；
    // kst: 接收保存文件状态的指针；
    // int ret = fstat(fd, &kst);
    // 返回值：成功返回0，失败返回-1；
    int fd = sysGetRealFd(context->a0);
    auto* stat = (struct kstat*) get_actual_page(context->a1);
    return fs->fstat(FD::GetFile(fd)->GetCStylePath(), stat);
}

size_t sys_fstatat(Context* context) {
    int dirfd=context->a0;
    auto *pathname= reinterpret_cast<const char *>(context->a1);
    auto *statbuf= reinterpret_cast<struct kstat *>(context->a2);
    int flags=context->a3;

    return 0;
}

size_t sys_lseek(Context* context){
    // int fd
    // size_t offset
    // int whence
    // size_t res = lseek(fd, offset, whence)
    // Upon successful completion, lseek() returns the resulting offset location as measured in bytes from the beginning of the file.  On error, the value (off_t) -1 is returned and errno is set to indicate the error
    int fd = sysGetRealFd(context->a0);
    return fs->lseek(FD::GetFile(fd)->GetCStylePath(), context->a1, context->a2);
}

size_t sys_exit_group(Context* context){
    return sys_exit(context);
}

size_t sys_readv(Context* context){
    // TODO: test
    int fd = sysGetRealFd(context->a0);
    auto *iov = (struct iovec*) get_actual_page(context->a1);
    int iovcnt = context->a2;
    int read_bytes = 0;
    int res;
    for(auto i = 0;i < iovcnt; i++){
        auto& bio = iov[i];
        res = fs->read(FD::GetFile(fd)->GetCStylePath(), (char *)get_actual_page((size_t)bio.iov_base), bio.iov_len);
        if(res != FR_OK)return -1;
        read_bytes += res;
    }
    return read_bytes;
}

size_t sys_writev(Context* context){
    // TODO: test
    int fd = sysGetRealFd(context->a0);
    auto *iov = (struct iovec*) get_actual_page(context->a1);
    int iovcnt = context->a2;
    int write_bytes = 0;
    int res;
    for(auto i = 0;i < iovcnt; i++){
        printf("[writev debug] %s\n",(char *)get_actual_page((size_t)iov[i].iov_base));
        auto& bio = iov[i];
        res = fs->write(FD::GetFile(fd)->GetCStylePath(), (char *)get_actual_page((size_t)bio.iov_base), bio.iov_len);
        if(res != FR_OK)return -1;
        write_bytes += res;
    }
    return write_bytes;
}

size_t sys_brk(Context* context){
    size_t require=context->a0;
//    printf("brk: require= 0x%x\n",require);
    return running->brk_control->brk(require);
}

size_t sys_getuid(Context* context){
    return 0;
}

size_t sys_geteuid(Context* context){
    return 0;
}

size_t sys_getgid(Context* context){
    return 0;
}

size_t sys_getegid(Context* context){
    return 0;
}

size_t sys_faccessat(Context* context){
    return 0;
}

size_t sys_gettid(Context* context){
    return 10000;
}

size_t sys_mmap(Context* context){
    // void *addr, size_t length, int prot, int flags, int fd, off_t offset
    void* addr= (void*)(context->a0); // addr is just a hint, so ignore
    size_t length=context->a1;
    int prot=context->a2;
    int flags=context->a3;
    int fd=context->a4;
    off_t offset=context->a5;

    return running->mmap_control->mmap(length);
}

size_t sys_munmap(Context* context){
    size_t addr=context->a0;
    size_t length=context->a1;

    return running->mmap_control->munmap(addr,length);
}

size_t sys_readlinkat(Context* context){
    int fd = (int) context->a0;
    const char* pathname= reinterpret_cast<const char *>(context->a1);
    char* buf= reinterpret_cast<char *>((context->a2));
    int length=context->a3;

    if(pathname != nullptr && strlen(pathname) > 0 && pathname[0] == '/'){
        String linkedFilePath = fs->ReadLink(pathname);
        strcpy(buf, linkedFilePath.c_str());
    }else{
        panic("un supported param in readlinkat")
    }
    return 0;
}

size_t sys_ioctl(Context* context){
    return 0;
}

size_t sys_rt_sigaction(Context* context){
    return 0;
}

size_t sys_kill(Context* context){
    int pid=context->a0;
    int signal=context->a1;

    List<PCB*>* list;
    auto* process=search_by_pid(&list,pid);
    if(process== nullptr){
        return -1;
    }
    process->kill(1);
    if(list){
        for(auto i=list->start;i;i=i->next){
            if(i->data==process){
                list->erase(i);
                break;
            }
        }
    }
    return 0;
}

size_t sys_sysinfo(Context* context){
    auto *info= reinterpret_cast<sysinfo *>(context->a0);

    info->uptime=114514;
    for(int i=0;i<3;i++){
        info->loads[i]=0;
    }
    info->totalram=1024*1024;
    info->freeram=114514;
    info->sharedram=114514;
    info->bufferram=114514;
    info->totalswap=114514;
    info->freeswap=114514;
    info->procs=100;
    return 0;
}

size_t sys_clock_gettime(Context* context){
    int clock_id=context->a0;
    auto *timespec= reinterpret_cast<struct timespec *>(context->a1);

    size_t* pc=reinterpret_cast<size_t*>(context->sepc);
    uint64 usec=timer();
    timespec->tv_sec=usec/1000000;
    timespec->tv_nsec=usec%1000000*1000;

    return 0;
}

size_t sys_getrusage(Context* context){
    int who=context->a0;
    struct rusage *usage= reinterpret_cast<struct rusage *>(context->a1);

    memset(usage,0,sizeof (rusage));

//    uint64 usec=timer();
//
//    usage->ru_stime.tv_sec=usec/1000000;
//    usage->ru_stime.tv_usec=usec%1000000;
//
//    usage->ru_utime.tv_sec=usec/1000000;
//    usage->ru_utime.tv_usec=usec%1000000;

    return 0;
}

size_t sys_utimensat(Context* context){
    int dir_fd = static_cast<int>(context->a0);
    char* pathname = reinterpret_cast<char*>(context->a1);
    auto* times = reinterpret_cast<struct timespec*>(context->a2);
    int flags = static_cast<int>(context->a0);

    char* path = atFdCWD(dir_fd, pathname);
    if(fs->fstat(path, nullptr) == -1){
        return -ENOENT;
    }else{
        panic("unsupported type in utimensat\n");
    }
    return 0;
}

size_t sys_fcntl(Context* context){
    int old_fd = sysGetRealFd((int) context->a0);
    size_t flag = context->a1;

    if (flag == F_GETFL) {
        return O_RDWR;
    }

    int new_fd = sysGetRealFd((int) context->a2);
    int actual_fd = FD::FindUnusedFdBiggerOrEqual(new_fd);
    FD::CopyFd(old_fd, actual_fd);
    return actual_fd;
}

size_t sys_sendfile(Context* context){
    int out_fd = context->a0;
    int in_fd = context->a1;
    auto* offset = (off_t*) context->a2;
    if(offset != nullptr){
        panic("unsupported sendfile arg")
    }
    size_t count = context->a3;
    char buff[count + 1];
    int read_bytes = fs->read(FD::GetFile(in_fd)->GetCStylePath(), buff, count);
    int write_bytes = fs->write(FD::GetFile(out_fd)->GetCStylePath(), buff, read_bytes);
    return write_bytes;
}

size_t sys_pipe2(Context* context){
    int* pipefd = reinterpret_cast<int*>(context->a0);
    int flags = context->a1;
    return FD::CreatePipe(pipefd, flags);
}

size_t sys_pselect6(Context* context) {
    int ndfs = (int) context->a0;
    auto* read_fds = (fd_set*) context->a1;
    auto* write_fds = (fd_set*) context->a2;
    auto* exception_fds = (fd_set*) context->a3;
    auto* timeout = (timespec*) context->a4;
    // sigset = context->a5

    if (exception_fds) {
        FD_ZERO(exception_fds);
    }

    if (read_fds) {
        fd_set read_fd_set_copy = *read_fds;
        FD_ZERO(read_fds);

        for (int i = 0;i < ndfs; i++) {
            if (FD_ISSET(i, &read_fd_set_copy)) {
                int res;
                res = fs->test_pipe(FD::GetFile(i)->GetCStylePath());
                if (res) {
                    FD_SET(i, read_fds);
                    return 1;
                }
            }
        }
    }

    return 0;
}

size_t sys_futex(Context* context) {
    auto* uaddr = (int*) context->a0;
    *uaddr = 0;
    return 0;
}

size_t sys_setitimer(Context* context) {
    return 0;
}

/// syscall int & register & distribute

void syscall_init() {
    for (int i = 0; i < SYSCALL_LIST_LENGTH; i++) {
        syscall_list[i] = nullptr;
        syscall_name_list[i] = nullptr;
    }
}

void syscall_unhandled(Context *context) {
    printf("[SYSCALL] Unhandled Syscall: %d\n", context->a7);
    context->a0=0;
}

void syscall_distribute(int syscall_id, Context *context) {
//#define STRACE
    static int syscall_is_initialized = 0;
    if (syscall_is_initialized != 1) {
        syscall_init();
        syscall_register();
        syscall_is_initialized = 1;
    }
    assert(syscall_id >= 0 && syscall_id < SYSCALL_LIST_LENGTH);
    assert(context != NULL);

    if (syscall_list[syscall_id] !=nullptr) {
#ifdef STRACE
        printf("[pid=%d] [syscall] %d, ",running->pid, syscall_id);
#endif
//        size_t int_start=timer();
        context->a0 = syscall_list[syscall_id](context);
//        size_t int_end=timer();
//        printf("<int cost %d>\n",(int_end-int_start));
        #ifdef STRACE
        printf("%s = 0x%x\n", syscall_name_list[syscall_id], context->a0);
#endif
    } else {
        syscall_unhandled(context);
    }
#undef STRACE
}

Context *syscall(Context *context) {
    syscall_distribute((int) context->a7, context);
    context->sepc += 4;
    return context;
}

void syscall_register() {
#define REGISTER(x) syscall_list[SYS_##x] = sys_##x, syscall_name_list[SYS_##x] = #x
    REGISTER(getchar);
    REGISTER(write);
    REGISTER(execve);
    REGISTER(exit);
    REGISTER(getppid);
    REGISTER(getpid);
    REGISTER(openat);
    REGISTER(read);
    REGISTER(close);
    REGISTER(getcwd);
    REGISTER(dup);
    REGISTER(dup3);
    REGISTER(chdir);
    REGISTER(getdents64);
    REGISTER(mkdirat);
    REGISTER(unlinkat);
    REGISTER(uname);
    REGISTER(wait4);
    REGISTER(clock_nanosleep);
    REGISTER(clone);
    REGISTER(sched_yield);
    REGISTER(mount);
    REGISTER(umount2);
    REGISTER(fstat);
    REGISTER(fstatat);
    REGISTER(lseek);
    REGISTER(readv);
    REGISTER(writev);
    REGISTER(brk);
    REGISTER(faccessat);
    REGISTER(getuid);
    REGISTER(geteuid);
    REGISTER(getgid);
    REGISTER(getegid);
    REGISTER(gettid);
    REGISTER(mprotect);
    REGISTER(exit_group);
    REGISTER(kill);
    REGISTER(sysinfo);
    REGISTER(mmap);
    REGISTER(munmap);
    REGISTER(readlinkat);
    REGISTER(ioctl);
    REGISTER(rt_sigaction);

    REGISTER(getrusage);
    REGISTER(clock_gettime);
    REGISTER(utimensat);
    REGISTER(fcntl);

    REGISTER(times);
    REGISTER(gettimeofday);
    REGISTER(sendfile);
    REGISTER(pipe2);
    REGISTER(pselect6);
    REGISTER(futex);
    REGISTER(setitimer);

#undef REGISTER
}
