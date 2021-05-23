# 编译须知
* 暂时需要将工具链的bin文件夹释放到环境变量
* 虽然cmake也能正常工作但是不建议使用，建议使用make

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