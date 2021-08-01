#include "kernel/interrupt.h"
#include "kernel/register.h"
#include "kernel/scheduler.h"
#include "kernel/self_test.h"
#include "kernel/memory/kernel_stack.h"
#include "kernel/times.h"
#include "lib/stl/string.h"
#include "kernel/fs/IFS.h"
#include "kernel/fs/VFS.h"
#include "kernel/Test.h"
#include "kernel/memory/memory.h"
#include "kernel/memory/Heap.h"
#include "kernel/fs/file_describer.h"
#include "lib/stl/Trie.h"
#include "lib/stl/PathUtil.h"
#include "lib/stl/RefCountPtr.h"
#include "lib/stl/UniquePtr.h"
#include "lib/stl/PageTableUtil.h"

void vfs_init();
void fix_kernel_page_table();

extern "C" {
#include "driver/interface.h"
#include "driver/fatfs/ff.h"
}

extern "C" void __cxa_pure_virtual() {
    // Do nothing or print an error message.
}

void test_lib() {
    TestString t = TestString();
    t.test();

    TestTrie testTrie;
    TestPathUtil testPathUtil;
    TestFile testFile;
}

/**
 * 此处打算通过 sret 进入u-mode
 * 中断后硬件会执行以下动作：
 * 异常指令的pc被保存在sepc中;pc被设置为stvec;
 * scause被设置为异常原因;stval被设置为出错地址或其他信息;
 * sie置零以禁止中断;之前的sie被保存到spie;
 * 之前的权限模式被保存到spp中;
 *
 * 之后的流程是我们手动完成的：保存现场;跳到中断处理函数;恢复现场;
 *
 * 所以这里需要恢复现场+将模拟硬件自动完成的动作。
 */
void init_thread() {
    printf("[OS] test library\n");
    test_lib();
    printf("[OS] times init.\n");
    init_times();
    printf("[OS] fix page table before driver\n");
    fix_kernel_page_table();
    printf("[OS] bsp init.\n");
    driver_init();
    printf("[FS] fs init.\n");
    vfs_init();
    printf("[OS] Interrupt & Timer Interrupt Open.\n");
    interrupt_timer_init();
    printf("[OS] init scheduler.\n");
    init_scheduler();
    init_file_describer();
    init_self_tests();

//    add_test("/stack_scan");
    add_test("/yield");
    add_test("/fork");
    add_test("/clone");
    add_test("/write");
    add_test("/uname");
    add_test("/times");
    add_test("/getpid");
    add_test("/getppid");
    add_test("/open");
    add_test("/read");
    add_test("/close");
    add_test("/openat");
    add_test("/getcwd");
    add_test("/dup");
    add_test("/getdents");
    add_test("/dup2");
    add_test("/wait");
    add_test("/exit");
    add_test("/execve");
    add_test("/gettimeofday");
    add_test("/mkdir_");
    add_test("/chdir");
    add_test("/waitpid");
    add_test("/sleep");
    add_test("/unlink");
    add_test("/mount");
    add_test("/umount");
    add_test("/fstat");

    // IO tests
//    add_test("/test_output");

    schedule();
}

void fix_kernel_page_table(){
    size_t table_base = register_read_satp() << 12;
    size_t virtual_address = 0x38001000;
    size_t physical_address = virtual_address;

    PageTableUtil::CreateMapping(table_base,
                                 virtual_address,
                                 physical_address,
                                 PAGE_TABLE_LEVEL::SMALL,
                                 PRIVILEGE_LEVEL::SUPERVISOR);
}

void vfs_init() {
    File::init();
    auto *ifs = new FS;
    ifs->init();
    fs = new VFS(ifs);

    auto *stdout = new StdoutFs;
    fs->root->appendChild(new File("dev", ifs));
    fs->mkdir("/dev", O_DIRECTORY | O_RDWR);

    auto *dev = fs->root->first_child->search("/dev");
    assert(dev != nullptr);
    dev->appendChild(new File("console", stdout));

    fs->root->appendChild(new File("sys", ifs));
    fs->mkdir("/sys", O_DIRECTORY | O_RDWR);

    auto* sys = fs->root->first_child->search("/sys");
    assert(sys != nullptr);

    sys->appendChild(new File("pipe", new PipeFs));
    fs->mkdir("/pipe", O_DIRECTORY | O_RDWR);

    auto* sys_pipe = fs->root->first_child->search("/sys/pipe");
    assert(sys_pipe != nullptr);
}

int main() {
    printf("   _____            _____               \n"
           "  / ____|          / ____|              \n"
           " | (___  _   _ ___| |     ___  _ __ ___ \n"
           "  \\___ \\| | | / __| |    / _ \\| '__/ _ \\\n"
           "  ____) | |_| \\__ \\ |___| (_) | | |  __/\n"
           " |_____/ \\__, |___/\\_____\\___/|_|  \\___|\n"
           "          __/ |                         \n"
           "         |___/                          \n");

    lty(__kernel_stack_base);

    printf("[OS] Memory Init.\n");
    init_memory();
//    init_kernel_heap();
    init_heap();
    kernelContext.kernel_satp = register_read_satp() | (8LL << 60);lty(kernelContext.kernel_satp);
    kernelContext.kernel_handle_interrupt = (size_t) handle_interrupt;
    kernelContext.kernel_restore = (size_t) __restore;

    init_thread();
    // unreachable
    panic("Unreachable code!");
    return 0;
}
