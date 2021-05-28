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
sudo test -d /mnt/sd || mkdir -p /mnt/sd
sudo mount -t vfat /dev/sda /mnt/sd
sudo cp -r test_suites/* /mnt/sd
sudo umount /mnt/sd
```

## 以下是一些有用的网站
* ![Risc-V Registers](https://www.five-embeddev.com/riscv-isa-manual/latest/supervisor.html#supervisor)
* ![K210 Datasheet](https://s3.cn-north-1.amazonaws.com.cn/dl.kendryte.com/documents/kendryte_datasheet_20180919020633.pdf)

## 鸣谢
rCore-Tutorial: https://rcore-os.github.io/rCore-Tutorial-Book-v3/index.html

xv6-k210: https://github.com/HUST-OS/xv6-k210

fatfs: http://elm-chan.org/fsw/ff/00index_e.html

rustsbi: https://github.com/luojia65/rustsbi