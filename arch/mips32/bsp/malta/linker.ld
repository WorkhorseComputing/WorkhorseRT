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

INCLUDE "autoconf.ld"

ENTRY(_start)

SECTIONS
{
    . = 0x80000000;
    __kernelStart = .;

    .boot.start : AT(ADDR(.boot.start))
    {
        . = ALIGN(4K);
        KEEP(*(.boot.start))
    }

    .boot.xTlb 0x80000080 : AT(0x80000080)
    {
        KEEP(*(.boot.xTlb))
    }

    .boot.interrupt 0x80000180 : AT(0x80000180)
    {
        KEEP(*(.boot.interrupt))
    }

    .boot.setup 0x80000200 : AT(0x80000200)
    {
        KEEP(*(.boot.setup))
    }

    . = ALIGN(4K);

    .text : AT(ADDR(.text))
    {
        . = ALIGN(4K);
        *(.text*)
    }

    .rodata : AT(ADDR(.rodata))
    {
        . = ALIGN(4K);
        *(.rodata*)
    }

    .data : AT(ADDR(.data))
    {
        . = ALIGN(4K);
        *(.data*)
    }

    .bss : AT(ADDR(.bss))
    {
        . = ALIGN(4K);
        __bssStart = .;
        *(COMMON)
        *(.bss*)
        __bssEnd = .;
    }

    /DISCARD/ :
    {
        *(.eh_frame)
        *(.eh_frame_hdr)
        *(.note.gnu.build-id)
    }

    .kInit : AT(ADDR(.kInit))
    {
        . = ALIGN(4K);
        __kInitStart = .;
        KEEP(*(SORT(.kInit.*)))
        __kInitEnd = .;
    }

    .kPlugin : AT(ADDR(.kPlugin))
    {
        . = ALIGN(4K);
        __kPluginStart = .;
        KEEP(*(SORT(.kPlugin.*)))
        __kPluginEnd = .;
    }

    __kernelEnd = .;
}

ASSERT((__kernelEnd - __kernelStart + __bootEnd) <= (CONFIG_MIPS32_KMAX_SIZE_MB * 1M), "kernel image too large");