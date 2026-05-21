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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.22)
project(WorkhorseRT C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include("${CMAKE_SOURCE_DIR}/generated/autoconf.cmake") 

link_directories("${CMAKE_SOURCE_DIR}/generated")

include("${CMAKE_SOURCE_DIR}/${CONFIG_CMAKE_TOOLCHAIN}") 

set(CMAKE_C_COMPILER   "${ARCH_TOOLCHAIN_C}")
set(CMAKE_CXX_COMPILER "${ARCH_TOOLCHAIN_CPP}")
set(CMAKE_ASM_COMPILER "${ARCH_TOOLCHAIN_ASM}")

add_executable(workhorse.elf "")

set_property(GLOBAL PROPERTY ARCH_INCLUDE_DIRS "")
set_property(GLOBAL PROPERTY ARCH_OBJS "")

add_subdirectory("${CMAKE_SOURCE_DIR}/${CONFIG_CMAKELISTS_SUBDIR}")
add_subdirectory(plugins)
add_subdirectory(src)

get_property(ALL_ARCH_INCLUDES GLOBAL PROPERTY ARCH_INCLUDE_DIRS)
get_property(ALL_ARCH_OBJS GLOBAL PROPERTY ARCH_OBJS)

include_directories(
    ${CMAKE_SOURCE_DIR}/include 
    ${ALL_ARCH_INCLUDES}
)

target_sources(workhorse.elf PRIVATE
    ${ALL_ARCH_OBJS}
    ${PLUGINS_OBJS}
    ${WORKHORSE_CORE_OBJS}
)

target_link_options(workhorse.elf PRIVATE -Wl,--no-relax)