cmake_minimum_required(VERSION 3.10)
project(os_risc_v CXX ASM)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -g -mcmodel=medany -T ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld -O2 -ffreestanding -nostdlib -O2 -ffreestanding -nostdlib")

include_directories(src/lib)

aux_source_directory(src/asm SRC_ASM)
aux_source_directory(src/driver SRC_DRIVER)
aux_source_directory(src/lib SRC_LIB)
aux_source_directory(src/driver/fatfs SRC_FATFS)

add_executable(
        os_risc_v
        ${SRC_ASM}
        ${SRC_DRIVER}
        ${SRC_LIB}
        ${SRC_FATFS}
        src/main.cpp)