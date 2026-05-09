/** MIT License
 *
 * Copyright (c) 2026 Humza Khan
 * <mohammed.khan.2024@uni.strath.ac.uk>
 * <https://github.com/humzak711>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef NULL
#   define NULL 0
#endif

#define UNREACHABLE() __builtin_unreachable()

#define STATIC_ASSERT(...) _Static_assert(__VA_ARGS__)

#define SIZE_ASSERT(obj, size) \
    STATIC_ASSERT(sizeof(obj) == (size), "size mismatch: " #obj)

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define ATTR_ALIGNED(x) __attribute__((aligned(x)))
#define ATTR_PACKED __attribute__((packed))
#define ATTR_NORETURN __attribute__((noreturn))
#define ATTR_HIDDEN __attribute__((visibility("hidden")));
#define ATTR_FALLTHROUGH __attribute__((fallthrough))

#define FEMTOSECOND 1000000000000000ULL

#define containerOf(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define returnAddress() __builtin_return_address(0)

#define TEST_BYTE 0x69

#ifndef barrier
#   define barrier() __asm__ __volatile__("": : :"memory")
#endif

#if defined(__i386__) || defined(__x86_64__)
#   define cpuRelax() __builtin_ia32_pause()

#   define cpuDisableInterrupts() __asm__ __volatile__ ("cli")
#   define cpuEnableInterrupts() __asm__ __volatile__ ("sti")

#   define cpuReadStatus() ({uint64_t rflags = 0; __asm__ __volatile__ ("pushfq; popq %0;":"=r"(rflags)); rflags;})
#   define cpuWriteStatus(status)  __asm__ __volatile__ ("pushq %0; popfq;"::"r"(status));

#else
#   define cpuRelax()
#endif

#define spinUntil(cond) do {    \
    while (!(cond))             \
        cpuRelax();             \
    } while (0)   

#endif