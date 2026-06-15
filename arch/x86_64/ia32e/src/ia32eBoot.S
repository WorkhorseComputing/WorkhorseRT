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

#define ASM_FILE 1

/* Includes */

#include <generated/autoconf.h>
#include <lib/multiboot2.h>
#include <ia32eAsm.h>
#include <ia32eCpu.h>

    .file "ia32eBoot.S"

/* Externs */

.extern __bssStart
.extern __bssEnd
.extern kStart
.extern ia32eCpuApStart

/* Definitions */

.globl _start
.type _start, @function

.globl start32
.type start32, @function

.globl ia32eSignature
.type ia32eSignature, @object

.globl ia32eMultiboot2Ptr
.type ia32eMultiboot2Ptr, @object

.globl ia32eApRsp
.type ia32eApRsp, @object

.globl ia32eApDr0
.type ia32eApDr0, @object

.globl ia32eIdt64
.type ia32eIdt64, @object

.globl ia32ePd
.type ia32ePd, @object

.globl ia32ePdpt
.type ia32ePdpt, @object

.globl ia32ePml4
.type ia32ePml4, @object

.globl ia32eWakeupBlobStart
.type ia32eWakeupBlobStart, @object

.globl ia32eWakeupBlobEnd
.type ia32eWakeupBlobEnd, @object

.globl ia32eWakeupBlobSaveArea
.type ia32eWakeupBlobSaveArea, @object

.globl ia32eBootStackPadding
.type ia32eBootStackPadding, @object

/* Boot Information */

.section .multiboot_header, "a", @progbits
.balign 64
header_start:
#if CONFIG_X86_64_IA32E_MULTIBOOT2

    .long MULTIBOOT2_HEADER_MAGIC
    .long 0
    .long header_end - header_start
    .long 0x100000000 - (MULTIBOOT2_HEADER_MAGIC + 0 + (header_end - header_start))

    .balign MULTIBOOT_TAG_ALIGN
    .word MULTIBOOT_TAG_TYPE_ELF_SECTIONS
    .word 0
    .long 16  
    .long 0 

    .balign MULTIBOOT_TAG_ALIGN
    .word MULTIBOOT_TAG_TYPE_MMAP      
    .word 0
    .long 16
    .long 24     
    .long 0     

    .balign MULTIBOOT_TAG_ALIGN
    .word MULTIBOOT_TAG_TYPE_END
    .word 0
    .long 8

    .balign MULTIBOOT_TAG_ALIGN
    .short MULTIBOOT_TAG_TYPE_FRAMEBUFFER
    .short 0 
    .long 20 
    .long 1024 
    .long 768
    .long 32 

#endif
header_end:

/* Boot Routine */

.section .boot.start

/* Boot entry */

.code32

.balign 64
_start:
    cli
    cld

    movl %eax, ia32eSignature
    movl %ebx, ia32eMultiboot2Ptr

    /* we are the bsp */

    movl $1, %edi

    jmp start32
    hlt

/* Protected Mode Entry */

.code32

.balign 64
start32:

    /* disable paging */

    movl %cr0, %eax
    orl $(IA32E_CR0_MP_MASK | IA32E_CR0_NE_MASK), %eax
    andl $(~(IA32E_CR0_EM_MASK | IA32E_CR0_NW_MASK | IA32E_CR0_CD_MASK | IA32E_CR0_PG_MASK)), %eax
    movl %eax, %cr0

    /* load cr3 */

    movl $ia32ePml4, %ebx
    movl %ebx, %cr3

    /* enable pae */

    movl $(IA32E_CR4_PAE_MASK | IA32E_CR4_PGE_MASK | IA32E_CR4_OSFXSR_MASK | IA32E_CR4_OSXMMEXCPT_MASK), %ecx
    movl %ecx, %cr4

    /* enable lme */

    movl $IA32E_EFER, %ecx
    rdmsr
    orl $IA32E_EFER_LONGMODE_ENABLE_MASK, %eax
    wrmsr

    /* enable paging and wp */

    movl %cr0, %ebx
    orl $(IA32E_CR0_WP_MASK | IA32E_CR0_PG_MASK), %ebx
    movl %ebx, %cr0

    /* setup gdtr and hand control to longmode */

    lgdt ia32eGdtr64
    ljmp $IA32E_KCS_SELECTOR, $start64

    hlt

/* Long Mode Entry */

.code64

.balign 64
start64:

    /* reload data selectors */

    movl $IA32E_KDS_SELECTOR, %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    movabsq $__ia32eStartHigh64, %rax
    jmpq *%rax

    hlt

/* Architectural data structures */

.section .lowdata

.balign 64
ia32eSignature:     .long 0
ia32eMultiboot2Ptr: .quad 0

ia32eGdt64Base:
    .quad           0
    .quad           IA32E_ASM_KCS_DESC
    .quad           IA32E_ASM_KDS_DESC
ia32eGdt64End:

ia32eGdtr64:
    .word           ia32eGdt64End - ia32eGdt64Base - 1
    .quad           ia32eGdt64Base

.balign 64
ia32eIdt64:         .zero 4096

.balign 64
ia32eIdtr64:
    .word           4095
    .quad           ia32eIdt64

IA32E_ASM_GEN_PD    ia32ePd
IA32E_ASM_GEN_PDPT  ia32ePdpt, ia32ePd 
IA32E_ASM_GEN_PML4  ia32ePml4, ia32ePdpt

/* Higher half kernel entry */

.section .text

.code64

.balign 64
__ia32eStartHigh64:

    /* load rsp based on whether we are bsp or ap */

    test %edi, %edi
    jz 2f

1:  leaq ia32eBootStackTop(%rip), %rsp 
    leaq ia32eBootStackPadding(%rip), %rsi

    /* zero bss before we touch rsp */

    xorl %eax, %eax
    leaq __bssEnd(%rip), %rcx
    leaq __bssStart(%rip), %rdi
    subq %rdi, %rcx
    rep stosb

    movl $1, %edi
    jmp 3f

2:  movq ia32eApRsp(%rip), %rsp
    movq ia32eApDr0(%rip), %rsi

3:  movq %rsp, %rbp

    /* set breakpoint to monitor the padding below the stack */

    movq %rsi, %dr0

    movq $IA32E_DR7_BP0_RW_8B_MASK, %rdx
    movq %rdx, %dr7

    /* null the ldtr */

    xorl %eax, %eax
    lldt %ax

    /* clean the floating point state */
    
    IA32E_ASM_RELOAD_FP_STATE

    /* load the boot idt */

    lidt ia32eIdtr64

    /* jump to c entry */

    push $0

    test %edi, %edi
    jz ia32eCpuApStart

    jmp kStart

    hlt

.section .data

/* AP wakeup */

.code16

.balign 64
ia32eWakeupBlobStart:

    cli
    cld

    lgdt IA32E_WAKEUP_ADDR + (ia32eGdtr32 - ia32eWakeupBlobStart)

    movl %cr0, %eax
    orl $(IA32E_CR0_PE_MASK), %eax
    movl %eax, %cr0

    xorl %edi, %edi
    ljmp $IA32E_KCS_SELECTOR, $(IA32E_WAKEUP_ADDR + (ia32eWakeup32 - ia32eWakeupBlobStart))

    hlt

.code32

.balign 64
ia32eWakeup32:

    movl $IA32E_KDS_SELECTOR, %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    movl $start32, %ecx
    jmpl *%ecx

    hlt

.balign 64
ia32eGdt32Base:
    .quad 0
    .quad 0x00CF9A000000FFFF
    .quad 0x00CF92000000FFFF
ia32eGdt32End:

ia32eGdtr32:
    .word ia32eGdt32End - ia32eGdt32Base - 1
    .long IA32E_WAKEUP_ADDR + (ia32eGdt32Base - ia32eWakeupBlobStart)

ia32eWakeupBlobEnd:

ia32eApRsp:                 .quad 0
ia32eApDr0:                 .quad 0

.section .bss

.balign 64
ia32eBootStackPadding:      .skip 16
ia32eBootStackBottom:
    .skip                   CONFIG_X86_64_IA32E_KSTACK_SIZE
ia32eBootStackTop:

.balign 64
ia32eWakeupBlobSaveArea:    .skip ia32eWakeupBlobEnd - ia32eWakeupBlobStart