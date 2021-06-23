# SysCore

* 2021全国大学生计算机系统能力大赛 - 操作系统赛- 内核实现赛道
* 学校：东北大学
* 参赛队伍：0x80200000
* iPear@https://github.com/i-Pear
* Waitti@https://github.com/Waitti-long

## Compile

```shell
# 将工具链bin目录释放至环境变量
```

```shell
# 编译出k210.bin
make
```

```shell
# 烧录到板子
make up
# 通过串口查看
make see
# 也可以直接make run，会完成从编译到烧录到串口查看的全过程
make run
```

<img src="doc\assets\make_run_demo.gif" alt="make_run_demo" style="zoom:60%;" />

## Img

```shell
# 构建fat.img
make img
```

## 格式化SD卡的命令

```shell
注意！你的sd卡究竟是不是/dev/sda!!!!!!!!!!!
sudo dd if=/dev/zero of=/dev/sda bs=1M count=1
sudo mkfs.vfat -F 32 /dev/sda
sudo test_lib -d /mnt/sd || mkdir -p /mnt/sd
sudo mount -t vfat /dev/sda /mnt/sd
sudo cp -r test_suites/* /mnt/sd
sudo umount /mnt/sd
```

## 如何在Syscore中使用qemu

首先，在src/driver中新建img.h并声明变量如下

```c
// 由于本代码过长请自己生成，此处无法完全写入MD也无法写入git
// 另外也不要把该文件上传到git上
unsigned char fs_img[] = {
0xeb, 0x58, 0x90, 0x6d, 0x6b, 0x66, 0x73, 0x2e, 0x66, 0x61, 0x74, 0x00,
```

编译driver

```sh
make qemu-driver
```

然后，下载qemu-6.0.0源码

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

运行qemu并监听1234端口
```shell
make qemu
```

再开一个shell运行gdb
```shell
make debug
```


## 以下是一些有用的网站

* ![Risc-V Registers](https://www.five-embeddev.com/riscv-isa-manual/latest/supervisor.html#supervisor)
* ![K210 Datasheet](https://s3.cn-north-1.amazonaws.com.cn/dl.kendryte.com/documents/kendryte_datasheet_20180919020633.pdf)

## 鸣谢

rCore-Tutorial: https://rcore-os.github.io/rCore-Tutorial-Book-v3/index.html

xv6-k210: https://github.com/HUST-OS/xv6-k210

fatfs: http://elm-chan.org/fsw/ff/00index_e.html

rustsbi: https://github.com/luojia65/rustsbi