//#pragma GCC optimize("Ofast")
#pragma GCC optimize("inline")
#pragma GCC optimize("-fdelayed-branch")
#pragma GCC optimize("-fguess-branch-probability")
#pragma GCC optimize("-fif-conversion")
#pragma GCC optimize("-fif-conversion2")
#pragma GCC optimize("-floop-optimize")
#pragma GCC optimize("-fmerge-constants")

#pragma GCC optimize("-fcaller-saves")
#pragma GCC optimize("-fcse-follow-jumps")
#pragma GCC optimize("-fdelete-null-pointer-checks")
#pragma GCC optimize("-fexpensive-optimizations")
#pragma GCC optimize("-fforce-mem")
#pragma GCC optimize("-fgcse")
#pragma GCC optimize("-fgcse-lm")
#pragma GCC optimize("-foptimize-sibling-calls")
#pragma GCC optimize("-fregmove")
#pragma GCC optimize("-freorder-functions")
#pragma GCC optimize("-frerun-cse-after-loop")
#pragma GCC optimize("-fsched-interblock")
#pragma GCC optimize("-fschedule-insns")
#pragma GCC optimize("-fstrength-reduce")
#pragma GCC optimize("-fthread-jumps")
#pragma GCC optimize("-funit-at-a-time")

#pragma GCC optimize("-fgcse-after-reload")
#pragma GCC optimize("-finline-functions")
#pragma GCC optimize("-finline-small-functions")
#pragma GCC optimize("no-stack-protector")
#pragma GCC optimize("unroll-loops")
#pragma GCC optimize("-ftree-loop-vectorize")
#pragma GCC optimize("-ftree-slp-vectorize")
#pragma GCC optimize("-fvect-cost-model")
#pragma GCC optimize("-ffast-math")

#include "kernel/interrupt.h"
#include "kernel/register.h"
#include "kernel/scheduler.h"
#include "kernel/self_test.h"
#include "kernel/memory/kernel_stack.h"
#include "lib/stl/string.h"
#include "kernel/fs/IFS.h"
#include "kernel/fs/VFS.h"
#include "kernel/Test.h"
#include "lib/stl/vector.h"
#include "kernel/memory/memory.h"
#include "kernel/memory/Heap.h"
#include "kernel/fs/file_describer.h"
#include "lib/stl/Trie.h"
#include "lib/stl/PathUtil.h"
#include "lib/stl/RefCountPtr.h"
#include "lib/stl/UniquePtr.h"
#include "lib/stl/PageTableUtil.h"
#include "kernel/time/time.h"
#include "kernel/posix/pselect.h"
#include "kernel/syscall.h"

void vfs_init();
void fix_kernel_page_table();

extern "C" {
#include "driver/interface.h"
#include "driver/fatfs/ff.h"
#include "driver/sysctl.h"
#include "driver/sleep/sleep.h"
#include "driver/rtc/rtc.h"
#include "driver/plic/plic.h"
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
//    printf("[OS] bsp init.\n");
    driver_init();
//    printf("[OS] Memory Init.\n");
    init_memory();
    init_heap();
    kernelContext.kernel_satp = register_read_satp() | (8LL << 60);
    kernelContext.kernel_handle_interrupt = (size_t) handle_interrupt;
    kernelContext.kernel_restore = (size_t) __restore;
//    printf("[OS] test library\n");
    test_lib();
    //    printf("[OS] rtc init.\n");
    //    init_rtc();
    //    printf("[OS] fix page table before driver\n");
    //    fix_kernel_page_table();
//    printf("[FS] fs init.\n");
    vfs_init();
//    printf("[OS] Interrupt & Timer Interrupt Open.\n");
    interrupt_timer_init();
//    printf("[OS] init scheduler.\n");
    init_scheduler();
    FD::InitializeFileDescriber();
    init_self_tests();
    init_syscall();

    schedule();
}

void vfs_init() {
    File::init();
    auto *ifs = new FS;
    ifs->init();
    fs = new VFS(ifs);

    auto* stdout = new StdoutFs;
    auto* zero_fs = new ZeroFs;
    auto* null_fs = new NullFs;
    auto* pipe_fs = new PipeFs;

    /** /dev **/
    fs->mkdir("/dev", 0, ifs);
    fs->mkdir("/dev/console", 0, stdout, false);
    fs->mkdir("/dev/zero", 0, zero_fs, false);
    fs->mkdir("/dev/null", 0, null_fs, false);

    /** /sys/pipe **/
    fs->mkdir("/sys", 0, ifs);
    fs->mkdir("/sys/pipe", 0, pipe_fs, false);

    /** /proc/self/exe **/
    fs->mkdir("/proc", 0, ifs);
    fs->mkdir("/proc/self", 0, ifs);
    fs->mkdir("/proc/self/exe", 0, ifs);
}

int main() {

//    printf("   _____            _____               \n"
//           "  / ____|          / ____|              \n"
//           " | (___  _   _ ___| |     ___  _ __ ___ \n"
//           "  \\___ \\| | | / __| |    / _ \\| '__/ _ \\\n"
//           "  ____) | |_| \\__ \\ |___| (_) | | |  __/\n"
//           " |_____/ \\__, |___/\\_____\\___/|_|  \\___|\n"
//           "          __/ |                         \n"
//           "         |___/                          \n");

    init_thread();
    // unreachable
    panic("Unreachable code!");
    return 0;
}
