#include "self_test.h"
#include "stdio.h"
#include "fs/VFS.h"

int test_cnt;
int test_total;
const char* tests[50];

int self_test_init_magic;

int has_next_test(){
    return test_cnt<test_total;
}

const char* get_next_test(){
    printf("Running \"%s\"\n",tests[test_cnt]);
    return tests[test_cnt++];
}

void simple_test(){
    //    add_test("/yield");
    //    add_test("/fork");
    //    add_test("/clone");
    //    add_test("/write");
    //    add_test("/uname");
    //    add_test("/times");
    //    add_test("/getpid");
    //    add_test("/getppid");
    //    add_test("/open");
    //    add_test("/read");
    //    add_test("/close");
    //    add_test("/openat");
    //    add_test("/getcwd");
    //    add_test("/dup");
    //    add_test("/getdents");
    //    add_test("/dup2");
    //    add_test("/wait");
    //    add_test("/exit");
    //    add_test("/execve");
    //    add_test("/gettimeofday");
    //    add_test("/mkdir_");
    //    add_test("/chdir");
    //    add_test("/waitpid");
    //    add_test("/sleep");
    //    add_test("/unlink");
    //    add_test("/mount");
    //    add_test("/umount");
    //    add_test("/fstat");
    //    add_test("/pipe");
}

void busybox_test(){

    // tested
    //    add_test("busybox_new ash -c exit");
    //    add_test("busybox_new sh -c exit");
    //    add_test("busybox_new du");
    //    add_test("busybox_new expr 1 + 1");
    //    add_test("busybox_new false");
    //    add_test("busybox_new true");
    //    add_test("busybox_new clear");
    //    add_test("busybox_new uname");
    //    add_test("busybox_new printf abc\n");
    //    add_test("busybox_new basename /aaa/bbb");
    //    add_test("busybox_new dirname /aaa/bbb");
    //    add_test("busybox_new echo 123");
    //    add_test("busybox_new printf \"abc\n\"");
    //    add_test("busybox_new echo \"#### independent command test\"");
    //    add_test("busybox_new sleep 1");
    //    add_test("busybox_new cal");
    //    add_test("busybox_new date");
    //    add_test("busybox_new kill 10");
    //    add_test("busybox_new pwd");

    // TODO: misc tests
    //    add_test("busybox_new df");         // too complex, skip
    //    add_test("busybox_new dmesg");      // too complex, skip
    //    add_test("busybox_new uptime");     // too complex, skip
    //    add_test("busybox_new free");       // need /proc/meminfo
    //    add_test("busybox_new ps");         // need /proc
    //    add_test("busybox_new hwclock");    // need /etc/localtime

    // passed fs tests
    //    add_test("busybox_new sh -c \"echo ccccccc > test.txt \"");
    //    add_test("busybox_new touch test.txt");
    //    add_test("busybox_new cat test.txt");
    //    add_test("busybox_new cut -c 3 test.txt");

    // TODO: file system tests
    //    add_test("busybox_new which ls");
    //    add_test("busybox_new ls");
    //    add_test("busybox_new sh -c \"echo ccccccc > test.txt \"");
    //    add_test("busybox_new cat test.txt");
    //    add_test("busybox_new cut -c 3 test.txt");
    //    add_test("busybox_new od test.txt");
    //    add_test("busybox_new head test.txt");
    //    add_test("busybox_new tail test.txt");
    //    add_test("busybox_new hexdump -C test.txt");
    //    add_test("busybox_new md5sum test.txt");
    //    add_test("busybox_new sort test.txt | ./busybox uniq");
    //    add_test("busybox_new stat test.txt");
    //    add_test("busybox_new strings test.txt");
    //    add_test("busybox_new wc test.txt");
    //    add_test("busybox_new [ -f test.txt ]");
    //    add_test("busybox_new more test.txt");
    //    add_test("busybox_new rm test.txt");
    //    add_test("busybox_new mkdir test_dir");
    //    add_test("busybox_new mv test_dir test");
    //    add_test("busybox_new rmdir test");
    //    add_test("busybox_new grep hello busybox_cmd.txt");
    //    add_test("busybox_new cp busybox_cmd.txt busybox_cmd.bak");
    //    add_test("busybox_new rm busybox_cmd.bak");
    //    add_test("busybox_new find -name \"busybox_cmd.txt\"");
}

void lua_test(){
    add_test("/lua-s sin30.lua");
}

void add_test(const char* name){
    tests[test_total++]=name;
}

void create_XXX_testfile(){
    fs->mkdir("/var", 0, fs->root->fs);
    fs->mkdir("/var/tmp", 0, fs->root->fs);
    File* XXX = VFS::search(fs->root, "/var/tmp/XXX");
    if (XXX) return;
    fs->open("/var/tmp/XXX", O_CREATE | O_RDWR);
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    for (int i = 0;i < 1024 * 7; i++) {
        fs->write("/var/tmp/XXX", buf, sizeof(buf));
    }
    fs->close("/var/tmp/XXX");
    printf("Create XXX test_file successfully.\n");
}

void create_lmbench_testfile(){
    File* LM = VFS::search(fs->root, "/var/tmp/lmbench");
    if (LM) return;
    fs->open("/var/tmp/lmbench", O_CREATE | O_RDWR);
    fs->close("/var/tmp/lmbench");
    printf("Create lmbench test_file successfully.\n");
}

void init_self_tests(){
    if(self_test_init_magic!=187439611){
        self_test_init_magic= 187439611;
        test_cnt=0;
        test_total=0;
        create_XXX_testfile();

        //    simple_test();
        //    busybox_test();
        //    lua_test();

        add_test("/lmbench_all lat_syscall -P 1 null");
        add_test("/lmbench_all lat_syscall -P 1 read");
        add_test("/lmbench_all lat_syscall -P 1 write");
//        add_test("/busybox mkdir -p /var/tmp");
//        add_test("/busybox touch /var/tmp/lmbench");
        add_test("/lmbench_all lat_syscall -P 1 stat /var/tmp/lmbench");
        add_test("/lmbench_all lat_syscall -P 1 fstat /var/tmp/lmbench");
        add_test("/lmbench_all lat_syscall -P 1 open /var/tmp/lmbench");

        add_test("/lmbench_all lat_pipe -P 1");
        add_test("/lmbench_all lat_sig -P 1 install");
        add_test("/lmbench_all lat_proc -P 1 fork");

        add_test("/lmbench_all lat_mmap -P 1 512k /var/tmp/XXX");
        add_test("/busybox echo Bandwidth measurements");
        add_test("/lmbench_all bw_mmap_rd -P 1 512k mmap_only /var/tmp/XXX");
        add_test("/lmbench_all bw_mmap_rd -P 1 512k open2close /var/tmp/XXX");
    }
}