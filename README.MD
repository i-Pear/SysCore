# SysCore

* 2021全国大学生计算机系统能力大赛 - 操作系统赛- 内核实现赛道

* 学校：东北大学

* 参赛队伍：0x80200000

* iPear@https://github.com/i-Pear

* Waitti@https://github.com/Waitti-long

* fluctlight001@https://github.com/fluctlight001

## 一、环境准备

  ```shell
# 将工具链bin目录释放至环境变量
export PATH=$PATH:/usr/your_toolchain_path
  ```

## 二、开发板运行


```shell
# 编译出k210.bin
make all
```

```shell
# 烧录到开发板
make up
# 通过串口查看
make see
# 也可以直接make run，会完成从编译到烧录到串口查看的全过程
make run
```

<img src="doc\assets\make_run_demo.gif" alt="make_run_demo" style="zoom:60%;" />

### 格式化SD卡的命令

```shell
！！！格式化前强检查SD卡是不是/dev/sda，否则可能格式化本机硬盘
sudo dd if=/dev/zero of=/dev/sda bs=1M count=20
sudo mkfs.vfat -F 32 /dev/sda
if [ ! -d "/mnt/sd" ]; then mkdir -p /mnt/sd; fi
sudo mount -t vfat /dev/sda /mnt/sd
sudo cp -r test_suites/* /mnt/sd
sudo umount /mnt/sd
```

## 三、QEMU模拟器运行

### 1. 安装QEMU

下载qemu-6.0.0源码

将target/riscv/cpu_helper.c中的以下代码注释掉，目的是使得任何的PTE_U都可以访问目标地址

```c
else if ((pte & PTE_U) && ((mode != PRV_U) &&
(!sum || access_type == MMU_INST_FETCH))) {
/* User PTE flags when not U mode and mstatus.SUM is not set,
   or the access type is an instruction fetch */
return TRANSLATE_FAIL;
} else if (!(pte & PTE_U) && (mode != PRV_S)) {
/* Supervisor PTE flags when not S mode */
return TRANSLATE_FAIL;
}
```

编译qemu

```sh
./configure --target-list=riscv64-softmmu,riscv64-linux-user
make -j$(nproc)
```

将build/riscv64-softmmu释放到环境变量

```sh
export PATH=$PATH:qemu-6.0.0/build/riscv64-softmmu
```

检查qemu版本

```shell
qemu-system-riscv64 --version
```

### 2. 准备SD卡内存镜像

首先，新建asserts文件夹并放入SD卡所需文件，执行命令：

```c
dd if=/dev/zero of=fs.img bs=1M count=20
mkfs.vfat -F 32 fs.img
sudo mount fs.img /mnt/sd
sudo cp -r ./assets/* /mnt/sd
sync
sudo umount /mnt/sd
xxd -i fs.img img.h
```

得到img.h文件，将其放入kernel/driver文件夹，编译driver：

```sh
make qemu-driver
```

### 3. 启动QEMU

运行qemu并使用1234端口进行debug

```shell
make dqemu
make debug # 再开一个shell运行gdb
```

也可以直接全速运行：

```makefile
make qemu
```

### 4. 加载符号

可以同时加载内核和用户态程序两个符号，享受双倍断点

```gdb
file build/kernel.o
add-symbol-file your_userspace_running_exectuable
```

## 四、一些有用的网站

* Risc-V Registers：https://www.five-embeddev.com/riscv-isa-manual/latest/supervisor.html#supervisor
* K210 Datasheet：https://s3.cn-north-1.amazonaws.com.cn/dl.kendryte.com/documents/kendryte_datasheet_20180919020633.pdf
* Linux Syscall：https://man7.org/linux/man-pages/man2/syscall.2.html
* AUX栈配置详解：https://man7.org/linux/man-pages/man3/getauxval.3.html
* glibc源码：https://code.woboq.org/userspace/glibc/
* VDSO相关：https://www.linuxjournal.com/content/creating-vdso-colonels-other-chicken

## 五、鸣谢

rCore-Tutorial: https://rcore-os.github.io/rCore-Tutorial-Book-v3/index.html

xv6-k210: https://github.com/HUST-OS/xv6-k210

fatfs: http://elm-chan.org/fsw/ff/00index_e.html

rustsbi: https://github.com/luojia65/rustsbi