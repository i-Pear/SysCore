BUILD = build
SRC = src

K210-SERIALPORT = /dev/ttyS5
K210-BURNER = platform/k210/kflash.py
BOOTLOADER = platform/k210/rustsbi-k210.bin
K210_BOOTLOADER_SIZE = 131072

KERNEL_BIN = k210.bin
KERNEL_O = $(BUILD)/kernel.o

GCC = riscv64-unknown-elf-gcc
GXX = riscv64-unknown-elf-c++
OBJCOPY = riscv64-unknown-elf-objcopy

SRC_ALL = $(wildcard src/vdso/*.h src/vdso/*.cpp src/lib/*.h src/lib/*.cpp src/lib/stl/*.h src/lib/stl/*.cpp src/kernel/*.h src/kernel/*.cpp src/kernel/memory/*.h src/kernel/memory/*.cpp src/kernel/fs/*.h src/kernel/fs/*.cpp src/kernel/posix/*.h src/kernel/posix/*.cpp src/kernel/time/*.h src/kernel/time/*.c src/kernel/time/*.cpp)
SRC_DRIVER = src/driver/all_driver_in_one.c
OBJ_DRIVER = $(BUILD)/driver.o
#SRC_FATFS = $(wildcard src/driver/fatfs/*.h src/driver/fatfs/*.c)

dst = /mnt/sd
sd = /dev/sda

# 在当前目录生成k210.bin
all: mk-build driver
	# gen build/
	$(GXX) -o $(KERNEL_O) -std=c++11 -w -g -mcmodel=medany -T src/linker.ld -ffreestanding -nostdlib -fno-exceptions -fno-rtti -Wwrite-strings -fno-use-cxa-atexit\
                                    $(SRC_ALL) \
                                    $(OBJ_DRIVER) \
                                    src/asm/boot.s \
                                    src/asm/interrupt.s \
                                    src/asm/fast-call.s \
                                    src/main.cpp
	$(OBJCOPY) $(KERNEL_O) --strip-all -O binary $(KERNEL_BIN)
	@cp $(BOOTLOADER) $(BOOTLOADER).copy
	@dd if=$(KERNEL_BIN) of=$(BOOTLOADER).copy bs=$(K210_BOOTLOADER_SIZE) seek=1
	@mv $(BOOTLOADER).copy $(KERNEL_BIN)

mk-build:
	@test -d $(BUILD) || mkdir -p $(BUILD)

driver:
	$(GCC) -o $(OBJ_DRIVER) -w -g -mcmodel=medany -ffreestanding -nostdlib -c $(SRC_DRIVER)

# 编译运行
run: all up see

# 烧录到板子
up:
	@sudo chmod 777 $(K210-SERIALPORT)
	python3 $(K210-BURNER) -p $(K210-SERIALPORT) -b 1500000 $(KERNEL_BIN)

# 通过串口查看
see:
	python3 -m serial.tools.miniterm --eol LF --dtr 0 --rts 0 --filter direct $(K210-SERIALPORT) 115200


img:
	@if [ ! -f "fs.img" ]; then \
		echo "making fs image..."; \
		dd if=/dev/zero of=fs.img bs=1M count=20; \
		mkfs.vfat -F 32 fs.img; fi
	@sudo mount fs.img $(dst)
	@sudo test -d $(dst) || mkdir -p $(dst)
	@sudo cp -r test_suites/* $(dst)
	@sudo umount $(dst)

flash:
	@sudo dd if=fs.img of=$(sd);

qemu-driver:
	@echo "Press enter to start..."
	@read REPLY
	@if [ ! -d "build" ]; then mkdir build; fi
	$(GCC) -o build/driver-qemu.o -w -g -mcmodel=medany -ffreestanding -nostdlib -c $(SRC_DRIVER) -DQEMU

qemu:
	$(GXX) -o $(KERNEL_O) -std=c++11 -w -g -mcmodel=medany -T src/linker-qemu.ld -ffreestanding -nostdlib -fno-exceptions -fno-rtti -Wwrite-strings -fno-use-cxa-atexit\
                                        $(SRC_ALL) \
                                        src/asm/boot.s \
                                        src/asm/interrupt-qemu.s \
                                        src/asm/fast-call.s \
                                        build/driver-qemu.o \
                                        src/main.cpp \
                                        -DQEMU
	$(OBJCOPY) $(KERNEL_O) --strip-all -O binary $(KERNEL_BIN)

	qemu-system-riscv64 -machine virt -nographic -bios platform/qemu/fw_payload.bin -rtc base=utc -device loader,file=k210.bin,addr=0x80200000 \
	-m 2000M -smp 2

dqemu:
	$(GXX) -o $(KERNEL_O) -std=c++11 -w -g -mcmodel=medany -T src/linker-qemu.ld -O0 -ffreestanding -nostdlib -fno-exceptions -fno-rtti -Wwrite-strings -fno-use-cxa-atexit\
                                        $(SRC_ALL) \
                                        src/asm/boot.s \
                                        src/asm/interrupt-qemu.s \
                                        src/asm/fast-call.s \
                                        build/driver-qemu.o \
                                        src/main.cpp \
                                        -DQEMU
	$(OBJCOPY) $(KERNEL_O) --strip-all -O binary $(KERNEL_BIN)

	qemu-system-riscv64 -machine virt -nographic -bios platform/qemu/fw_payload.bin -rtc base=utc -device loader,file=k210.bin,addr=0x80200000 \
	-S -s -m 2000M -smp 2

debug:
	riscv64-unknown-elf-gdb build/kernel.o -ex "target remote :1234"