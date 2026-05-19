#[[ MIT License

Copyright (c) 2026 Humza Khan
<mohammed.khan.2024@uni.strath.ac.uk>
<https://github.com/humzak711>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
]]

set(CMAKE_C_COMPILER x86_64-elf-gcc)

set(CMAKE_EXE_LINKER_FLAGS
    "-nostdlib -no-pie -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/linker.ld"
    PARENT_SCOPE
)

set(ARCH_INCLUDE_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PARENT_SCOPE
)

set(ARCH_FLAGS PRIVATE
    -O2
    -mstackrealign
    -ffreestanding
    -Wall
    -Wextra
    -fno-stack-protector
    -m64
    -msse
    -msse2
    -fPIE
    -fPIC
)

file(GLOB ARCH_IA32E_C
    src/*.c
)

file(GLOB ARCH_IA32E_ASM
    src/*.S
)

set(ARCH_OBJS
    ${ARCH_IA32E_C}
    ${ARCH_IA32E_ASM}
    PARENT_SCOPE
)

target_compile_options(workhorse.elf PRIVATE ${ARCH_FLAGS})

set(OUTPUT_ISO ${CMAKE_BINARY_DIR}/workhorse.iso)

add_custom_target(iso
    COMMAND  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/boot/ia32eMakeIso.sh
            ${CMAKE_BINARY_DIR}/workhorse.elf
            ${OUTPUT_ISO}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS workhorse.elf
)

add_custom_target(run
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/emulate/ia32eQemu.sh
            ${OUTPUT_ISO}
            512M
            ${CONFIG_KMAX_CPUS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS iso
)

add_custom_target(runKvm
    COMMAND  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/emulate/ia32eQemuKvm.sh
            ${OUTPUT_ISO}
            512M
            ${CONFIG_KMAX_CPUS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS iso
)