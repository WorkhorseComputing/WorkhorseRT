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
#include <ia32eAsm.h>
#include <ia32eCpu.h>

    .file "ia32eEntry.S"

/* Externs */

.extern ia32eIsrPreHandler
.extern ia32eIsrHandler
.extern ia32eSyscallHandler

#if CONFIG_IA32E_VTX

.extern ia32eEmulatorBootManager
.extern ia32eEmulatorVcpuFailureEntry
.extern ia32eEmulatorDispatcher

#endif 

/* Definitions */

#define IA32E_ASM_ISR_ENTRY_PREFIX __ia32eIsrEntry_

.globl ia32eIsrEntryTable
.type ia32eIsrEntryTable, @object

.globl __ia32eSyscallEntry
.type __ia32eSyscallEntry, @function

.globl __ia32eTaskIdleEntry
.type __ia32eTaskIdleEntry, @function

.globl __ia32eFakeIsr
.type __ia32eFakeIsr, @function

#if CONFIG_IA32E_VTX

.globl __ia32eVmlaunchStub
.type __ia32eVmlaunchStub, @function

.globl __ia32eVmexitStub
.type __ia32eVmexitStub, @function

#endif

/* Body */

.section .text

.code64

/* Isr entry rendezvous */

.balign 64
__ia32eIsrEntry:

    cld
    IA32E_ASM_PUSH_FRAME

    IA32E_ASM_RELOAD_FP_STATE

    movq %rsp, %rdi
    call ia32eIsrPreHandler

    /* Call the handler, which replies with the next state transition -
       are we to swapgs or not? */

    movq %rsp, %rdi
    call ia32eIsrHandler
    test %eax, %eax
    jz 1f

    swapgs

1:  IA32E_ASM_POP_FRAME
    iretq

    int3

/* Isr entries */

IA32E_ASM_GEN_ISR_ENTRIES IA32E_ASM_ISR_ENTRY_PREFIX, __ia32eIsrEntry

/* Syscall entry */

.balign 64
__ia32eSyscallEntry:

    /* Switch to the kernel gs, this is required before enabling interruptd as our
       context switcher decides to swapgs based on whether we were in kernel or user
       context at the time of preemption */
    swapgs
    sti

    /* Save the users stack ptr, switch to the syscall stack ptr */

    movq %rsp, %gs:IA32E_ASM_SYSCALL_USP_OFF
    movq %gs:IA32E_ASM_SYSCALL_KSP_OFF, %rsp

    /* Save the users regs on the stack, call into our handler then restore the regs */

    IA32E_ASM_PUSH_REGS

    IA32E_ASM_RELOAD_FP_STATE

    movq %rsp, %rdi
    call ia32eSyscallHandler

    IA32E_ASM_POP_REGS

    /* Restore the users current stack ptr */

    movq %gs:IA32E_ASM_SYSCALL_USP_OFF, %rsp

    /* Hand back control to the user */

    cli
    swapgs
    sysretq

    int3

.balign 64
__ia32eTaskIdleEntry:
    
    sti

1:  hlt
    jmp 1b

    int3

/* This expects interrupts to be disabled beforehand */

.balign 64
__ia32eFakeIsr:

    movzx %sil, %esi

    movq %rsp, %rcx
    movq %rdi, %rsp

    movl %ss, %edi
    pushq %rdi

    pushq %rcx
    pushfq

    movl %cs, %edi
    pushq %rdi

    leaq 1f(%rip), %rdi
    pushq %rdi

    pushq %rdx
    pushq %rsi

    jmp __ia32eIsrEntry

1:  .byte 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
    ret

    int3

/* Vtx stubs */

#if CONFIG_IA32E_VTX


.balign 64
__ia32eVmlaunchStub:
 
    call ia32eEmulatorBootManager

    xorl %eax, %eax
    xorl %ebx, %ebx
    xorl %ecx, %ecx
    xorl %edx, %edx
    xorl %edi, %edi
    xorl %esi, %esi
    xorl %ebp, %ebp
    xorl %r8d, %r8d
    xorl %r9d, %r9d
    xorl %r10d, %r10d
    xorl %r11d, %r11d
    xorl %r12d, %r12d
    xorl %r13d, %r13d
    xorl %r14d, %r14d
    xorl %r15d, %r15d

    pushq $0x1f80
    ldmxcsr (%rsp)
    addq $8, %rsp

    pxor %xmm0, %xmm0 
    pxor %xmm1, %xmm1 
    pxor %xmm2, %xmm2 
    pxor %xmm3, %xmm3 
    pxor %xmm4, %xmm4 
    pxor %xmm5, %xmm5 
    pxor %xmm6, %xmm6 
    pxor %xmm7, %xmm7 
    pxor %xmm8, %xmm8 
    pxor %xmm9, %xmm9 
    pxor %xmm10, %xmm10 
    pxor %xmm11, %xmm11 
    pxor %xmm12, %xmm12 
    pxor %xmm13, %xmm13 
    pxor %xmm14, %xmm14 
    pxor %xmm15, %xmm15

    fninit
    fldz
    fst %st(1) 
    fst %st(2) 
    fst %st(3) 
    fst %st(4) 
    fst %st(5) 
    fst %st(6) 
    fst %st(7) 
    fstp %st(0)

    movq %rax, %cr2
    
    movl $0xffff0ff0, %eax
    movq %rax, %dr6
    xorl %eax, %eax

    movq %rax, %dr3
    movq %rax, %dr2
    movq %rax, %dr1
    movq %rax, %dr0

    vmlaunch 
    jmp 1f

.balign 64
__ia32eVmexitStub:
    
    IA32E_ASM_PUSH_REGS

    movq %dr6, %rax 
    pushq %rax

    movq %dr3, %rax 
    pushq %rax

    movq %dr2, %rax 
    pushq %rax

    movq %dr1, %rax 
    pushq %rax

    movq %dr0, %rax 
    pushq %rax

    movq %cr2, %rax 
    pushq %rax

    IA32E_ASM_RELOAD_FP_STATE

    movq %rsp, %rdi 
    call ia32eEmulatorDispatcher

    cli

    popq %rax 
    movq %rax, %cr2

    popq %rax 
    movq %rax, %dr0
    
    popq %rax 
    movq %rax, %dr1

    popq %rax 
    movq %rax, %dr2

    popq %rax 
    movq %rax, %dr3

    popq %rax 
    movq %rax, %dr6

    IA32E_ASM_POP_REGS

    vmresume
    IA32E_ASM_RELOAD_FP_STATE

/* vmentry failure stub */

1:  call ia32eEmulatorVcpuFailureEntry
    int3

#endif

/* Isr entry table */

.section .data

.balign 64
ia32eIsrEntryTable: IA32E_ASM_GEN_ISR_TABLE IA32E_ASM_ISR_ENTRY_PREFIX