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

KERNEL_OFFSET   = 0xffffff8000000000;
BOOT_ADDR       = 1M;

SECTIONS
{
    __bootStart = .;
    . = BOOT_ADDR;

    .boot :
    {
        *(.multiboot_header)
        *(.boot*)
        *(.lowdata*)
    }
    __bootEnd = ALIGN(4K, BOOT_ADDR + SIZEOF(.boot));

    KERNEL_ADDR = __bootEnd;
    KERNEL_VIRTUAL_ADDR = KERNEL_ADDR + KERNEL_OFFSET;

    . = KERNEL_VIRTUAL_ADDR;
    __kernelStart = .;

    .text : AT(ADDR(.text) - KERNEL_VIRTUAL_ADDR + KERNEL_ADDR)
    {
        . = ALIGN(4K);
        *(.text*)
    }

    .rodata : AT(ADDR(.rodata) - KERNEL_VIRTUAL_ADDR + KERNEL_ADDR)
    {
        . = ALIGN(4K);
        *(.rodata*)
    }

    .data : AT(ADDR(.data) - KERNEL_VIRTUAL_ADDR + KERNEL_ADDR)
    {
        . = ALIGN(4K);
        *(.data*)
    }

    .bss : AT(ADDR(.bss) - KERNEL_VIRTUAL_ADDR + KERNEL_ADDR)
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

    .ia32eVma : AT(ADDR(.ia32eVma) - KERNEL_VIRTUAL_ADDR + KERNEL_ADDR)
    {
        . = ALIGN(4K);
        __ia32eVmaStart = .;
        KEEP(*(.ia32eVma))
        __ia32eVmaEnd = .;
    }

    .kInit : AT(ADDR(.kInit) - KERNEL_VIRTUAL_ADDR + KERNEL_ADDR)
    {
        . = ALIGN(4K);
        __kInitStart = .;
        KEEP(*(SORT(.kInit.*)))
        __kInitEnd = .;
    }

    .kPlugin : AT(ADDR(.kPlugin) - KERNEL_VIRTUAL_ADDR + KERNEL_ADDR)
    {
        . = ALIGN(4K);
        __kPluginStart = .;
        KEEP(*(SORT(.kPlugin.*)))
        __kPluginEnd = .;
    }

    __kernelEnd = .;
}

ASSERT((__kernelEnd - __kernelStart + __bootEnd) <= (CONFIG_X86_64_IA32E_KMAX_SIZE_MB * 1M), "kernel image too large");