cmake_minimum_required(VERSION 3.10)
project(os_risc_v CXX ASM)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -g -mcmodel=medany -T ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld -O2 -ffreestanding -nostdlib -O2 -ffreestanding -nostdlib")

include_directories(src/kernel)

aux_source_directory(src/asm SRC_ASM)
aux_source_directory(src/driver SRC_DRIVER)
aux_source_directory(src/kernel SRC_LIB)
aux_source_directory(src/kernel/memory SRC_MEMORY)
aux_source_directory(src/kernel/time SRC_RTC)
aux_source_directory(src/kernel/fs SRC_FS)
aux_source_directory(src/kernel/posix SRC_POSIX)
aux_source_directory(src/driver/fatfs SRC_FATFS)

add_executable(
        os_risc_v
        ${SRC_ASM}
        ${SRC_DRIVER}
        ${SRC_LIB}
        ${SRC_FATFS}
        ${SRC_RTC}
        ${SRC_FS}
        ${SRC_MEMORY}
        ${SRC_POSIX}
        src/main.cpp)