cmake_minimum_required(VERSION 3.10)
project(os_risc_v C ASM)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -g -mcmodel=medany -T ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld -O2 -ffreestanding -nostdlib -O2 -ffreestanding -nostdlib")

include_directories(src/lib)

add_executable(
        os_risc_v
        src/asm/boot.s
        src/asm/interrupt.s
        src/lib/interrupt.c
        src/lib/interrupt.h
        src/lib/math.c
        src/lib/math.h
        src/lib/memory.c
        src/lib/memory.h
        src/lib/sbi.c
        src/lib/sbi.h
        src/lib/stdarg.h
        src/lib/stdbool.h
        src/lib/stddef.h
        src/lib/stdio.c
        src/lib/stdio.h
        src/lib/stl.h
        src/lib/stl.c
        src/lib/page.c
        src/lib/page.h
        src/main.c
        )