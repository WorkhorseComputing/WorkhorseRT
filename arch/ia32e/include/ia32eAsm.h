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

#ifndef _IA32E_ASM_H_
#define _IA32E_ASM_H_

#include <generated/autoconf.h>
#include <ia32e.h>

#define IA32E_KERNEL_OFFSET 0xffffff8000000000
#define IA32E_WAKEUP_VECTOR 7ULL
#define IA32E_WAKEUP_ADDR (IA32E_WAKEUP_VECTOR << 12)
#define IA32E_FREE_PDPTE_START 1

#define IA32E_DEFAULT_FCW 0x037f
#define IA32E_DEFAULT_MXCSR 0x1f80

#define IA32E_NUM_VECTORS 256
#define IA32E_NUM_VECTOR_PRIO 16
#define IA32E_MIN_VECTOR_PRIO 0
#define IA32E_MAX_VECTOR_PRIO 15
#define IA32E_VECTOR_TO_PRIO(vec) ((vec) / 16)

#define IA32E_SELECTOR_TO_RPL(sel) ((sel) & 0x3)

#define IA32E_ASM_KCS_DESC          0x00af9b000000ffff
#define IA32E_ASM_KDS_DESC          0x00af93000000ffff
#define IA32E_ASM_UCS_DESC          0x00affb000000ffff
#define IA32E_ASM_UDS_DESC          0x00aff3000000ffff

#define IA32E_KCS_IDX       1ULL
#define IA32E_KDS_IDX       2ULL
#define IA32E_UCS_IDX       4ULL
#define IA32E_UDS_IDX       3ULL
#define IA32E_TR_LOW_IDX    5ULL
#define IA32E_TR_HIGH_IDX   6ULL

#define IA32E_KCS_SELECTOR  (IA32E_KCS_IDX << 3)
#define IA32E_KDS_SELECTOR  (IA32E_KDS_IDX << 3)
#define IA32E_UCS_SELECTOR  ((IA32E_UCS_IDX << 3) | 3)
#define IA32E_UDS_SELECTOR  ((IA32E_UDS_IDX << 3) | 3)
#define IA32E_TR_SELECTOR   (IA32E_TR_LOW_IDX << 3)

#define IA32E_INVPCID_ADDR              0
#define IA32E_INVPCID_CTX               1
#define IA32E_INVPCID_ALL_INC_GLOBAL    2
#define IA32E_INVPCID_ALL_NO_GLOBAL     3

#ifdef ASM_FILE

.macro IA32E_ASM_GEN_ISR_ENTRY, ISR_NO, prefix, entry_func
    .balign 16
    \prefix\ISR_NO:
        pushq $0
        pushq $\ISR_NO
        jmp \entry_func
.endm

.macro IA32E_ASM_GEN_ISR_ENTRY_ERRCODE, ISR_NO, prefix, entry_func
    .balign 16
    \prefix\ISR_NO:
        pushq $\ISR_NO
        jmp \entry_func
.endm

.altmacro
.macro IA32E_ASM_GEN_ISR_ENTRIES, prefix, entry_func

    LOCAL i
    .set i, 0

    .rept IA32E_NUM_VECTORS
        .if i == 8 || i == 10 || i == 11 || i == 12 || i == 13 || i == 14 || i == 17 || i == 21
            IA32E_ASM_GEN_ISR_ENTRY_ERRCODE %(i), \prefix, \entry_func
        .else
            IA32E_ASM_GEN_ISR_ENTRY %(i), \prefix, \entry_func
        .endif

        .set i, i+1
    .endr

.endm

.macro IA32E_ASM_GEN_ISR_TABLE_ENTRY, ISR_NO, prefix
    .quad \prefix\ISR_NO
.endm

.altmacro
.macro IA32E_ASM_GEN_ISR_TABLE, prefix

    LOCAL i
    .set i, 0

    .rept IA32E_NUM_VECTORS
        IA32E_ASM_GEN_ISR_TABLE_ENTRY %(i), \prefix
        .set i, i+1
    .endr
    
.endm

.macro IA32E_ASM_PUSH_REGS
    subq $512, %rsp
    fxsave (%rsp)

    pushq %rbp
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rdi
    pushq %rsi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    subq $8, %rsp
.endm

.macro IA32E_ASM_POP_REGS
    addq $8, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rsi
    popq %rdi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popq %rbp

    fxrstor (%rsp)
    addq $512, %rsp
.endm

.macro IA32E_ASM_PUSH_FRAME
    push $0
    IA32E_ASM_PUSH_REGS
.endm

.macro IA32E_ASM_POP_FRAME
    IA32E_ASM_POP_REGS
    addq $24, %rsp
.endm

.macro IA32E_ASM_GEN_PD name
    .balign 4096
    \name:
        .set addr, 0
        .set numEntries, (CONFIG_IA32E_KMAX_SIZE_MB + 1) / 2
        .rept numEntries
            .quad addr + 0x183  /* P | RW | PS | G */
            .set addr, addr + (2 * 1024 * 1024)
        .endr
.endm

.macro IA32E_ASM_GEN_PDPT name, pd
    .balign 4096
    \name:
        .quad (\pd) + 0x103   /* P | RW | G */

        .rept (511)
            .quad 0
        .endr
.endm

.macro IA32E_ASM_GEN_PML4 name, pdpt
    .balign 4096
    \name:
        .quad \pdpt + 0x103 /* P | RW | G */

        .rept 510
            .quad 0
        .endr

        .quad \pdpt + 0x103 
.endm

#else

#include <generated/autoconf.h>
#include <compiler.h>

#define ia32eRdtsc() __builtin_ia32_rdtsc()

typedef struct ia32eStack
{
    char padding[16] ATTR_ALIGNED(16);
    char stack[CONFIG_IA32E_KSTACK_SIZE] ATTR_ALIGNED(16);
} ATTR_PACKED ia32eStack_t;
SIZE_ASSERT(ia32eStack_t, CONFIG_IA32E_KSTACK_SIZE + 16);

typedef struct ATTR_PACKED ia32eRegs
{
    uint64_t alignment;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t rbp;
    ia32eFxsave64_t fxsaveRegion;
} ia32eRegs_t;
SIZE_ASSERT(ia32eRegs_t, (16 * sizeof(uint64_t)) + 512);

typedef struct ATTR_PACKED ia32eFrame
{
    ia32eRegs_t regs;
    uint64_t resvd0;

    uint64_t vector;
    uint64_t errcode;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} ia32eFrame_t;
SIZE_ASSERT(ia32eFrame_t, sizeof(ia32eRegs_t) + (8 * sizeof(uint64_t)));

typedef struct ATTR_PACKED ia32eInvpcidDesc
{
    uint64_t pcid;
    uint64_t addr;
} ia32eInvpcidDesc_t;
SIZE_ASSERT(ia32eInvpcidDesc_t, 16);

inline
void __ia32eSwapgs(void)
{
    __asm__ __volatile__ ("swapgs");
}

inline 
void __ia32eCpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    __asm__ __volatile__ (
        "cpuid"
        :"=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        :"a"(*eax), "c"(*ecx)
    );
}

inline 
void ia32eCpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    __asm__ __volatile__ (
        "cpuid"
        :"=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        :"a"(leaf), "c"(subleaf)
    );
}

inline 
void __ia32eWrmsr(uint32_t msr, uint64_t val)
{
    uint64_t low = val & 0xffffffff;
    uint64_t high = val >> 32;

    __asm__ __volatile__ ("wrmsr" ::"c"(msr), "a"(low), "d"(high));
}

inline 
uint64_t __ia32eRdmsr(uint32_t msr)
{
    uint32_t low = 0;
    uint32_t high = 0;
    __asm__ __volatile__ ("rdmsr" :"=a"(low), "=d"(high) :"c"(msr));

    return ((uint64_t)high << 32) | low;
}

inline 
uint8_t __ia32eInb(uint16_t port)
{
    uint8_t ret = 0;
    __asm__ __volatile__ ("inb %1, %0" :"=a"(ret) :"d"(port));
    return ret;
}

inline
void __ia32eHlt(void)
{
    __asm__ __volatile__ ("hlt");
}

inline 
void __ia32eHltForever(void)
{
    while (1)
        __ia32eHlt();

    UNREACHABLE();
}

inline 
uint64_t __ia32eReadCr0(void)
{
    uint64_t cr0 = 0;
    __asm__ __volatile__ ("mov %%cr0, %%rax" :"=a"(cr0));
    return cr0;
}

inline 
uint64_t __ia32eReadCr2(void)
{
    uint64_t cr2 = 0;
    __asm__ __volatile__ ("mov %%cr2, %%rax" :"=a"(cr2));
    return cr2;
}

inline 
uint64_t __ia32eReadCr3(void)
{
    uint64_t cr3 = 0;
    __asm__ __volatile__ ("mov %%cr3, %%rax" :"=a"(cr3));
    return cr3;
}

inline 
uint64_t __ia32eReadCr4(void)
{
    uint64_t cr4 = 0;
    __asm__ __volatile__ ("mov %%cr4, %%rax" :"=a"(cr4));
    return cr4;
}

inline 
uint64_t __ia32eReadCr8(void)
{
    uint64_t cr8 = 0;
    __asm__ __volatile__ ("mov %%cr8, %%rax" :"=a"(cr8));
    return cr8;
}

inline 
void __ia32eWriteCr0(uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%cr0" :: "r"(val));
}

inline 
void __ia32eWriteCr3(uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%cr3" :: "r"(val));
}

inline 
void __ia32eWriteCr4(uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%cr4" :: "r"(val));
}

inline 
void __ia32eWriteCr8(uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%cr8" :: "r"(val));
}


inline 
uint16_t __ia32Inw(uint16_t port)
{
    uint16_t ret = 0;
    __asm__ __volatile__ ("inw %1, %0" :"=a"(ret) :"d"(port));
    return ret;
}

inline 
uint32_t __ia32eInl(uint16_t port)
{
    uint32_t ret = 0;
    __asm__ __volatile__ ("inl %1, %0" :"=a"(ret) :"d"(port));
    return ret;
}

inline 
void __ia32eOutb(uint16_t port, uint8_t val)
{
    __asm__ __volatile__ ("outb %0, %1" ::"a"(val), "d"(port));
}

inline 
void __ia32eOutw(uint16_t port, uint16_t val)
{
    __asm__ __volatile__ ("outw %0, %1" ::"a"(val), "d"(port));
}

inline 
void __ia32eOutl(uint16_t port, uint32_t val)
{
    __asm__ __volatile__ ("outl %0, %1" ::"a"(val), "d"(port));
}

inline
void __ia32ePause(void)
{
    __asm__ __volatile__ ("pause");
}

inline 
uint64_t __ia32eRdtsc(void)
{
    uint32_t eax = 0;
    uint32_t edx = 0;

    __asm__ __volatile__ ("rdtsc;":"=a"(eax), "=d"(edx));

    return ((uint64_t)edx << 32) | eax;
}

inline 
uint32_t __ia32eReadgs32(uint32_t offset)
{
    uint32_t val = 0;
    __asm__ __volatile__ ("movl %%gs:(%1), %0":"=r"(val) :"r"(offset));
    return val;
}

inline 
uint64_t __ia32eReadgs64(uint32_t offset)
{
    uint64_t val = 0;
    __asm__ __volatile__ ("movq %%gs:(%1), %0":"=r"(val) :"r"(offset));
    return val;
}

inline 
void __ia32eWritegs64(uint32_t offset, uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%gs:(%1)"::"r"(val), "r"(offset));
}

inline 
uint32_t __ia32eReadfs32(uint32_t offset)
{
    uint32_t val = 0;
    __asm__ __volatile__ ("movl %%fs:(%1), %0":"=r"(val) :"r"(offset));
    return val;
}

inline
uint64_t __ia32eReadfs64(uint32_t offset)
{
    uint64_t val = 0;
    __asm__ __volatile__ ("movq %%fs:(%1), %0":"=r"(val) :"r"(offset));
    return val;
}

inline 
void __ia32eWritefs64(uint32_t offset, uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%fs:(%1)"::"r"(val), "r"(offset));
}

inline 
void __ia32eFxsave(void *addr)
{
    __asm__ __volatile__ ("fxsave (%0)"::"r"(addr):"memory");
}

inline 
void __ia32eFxrstore(void *addr)
{
    __asm__ __volatile__ ("fxrstore (%0)"::"r"(addr):"memory");
}

inline 
void __ia32eLtr(uint16_t tr)
{
    __asm__ __volatile__ ("ltr %0"::"r"(tr));
}

inline
void __ia32eCr4ReenablePge(void)
{
    uint64_t cr4 = 0;

    cr4 = __ia32eReadCr4();

    cr4 &= ~(IA32E_CR4_PGE_MASK);
    __ia32eWriteCr4(cr4);

    cr4 |= IA32E_CR4_PGE_MASK;
    __ia32eWriteCr4(cr4);
}

inline 
void __ia32eLidt(ia32eDescriptorReg64_t *idtr)
{
    __asm__ __volatile__ ("lidt %0;"::"m"(*idtr) :"memory");
}

inline 
void __ia32eLgdt(ia32eDescriptorReg64_t *gdtr)
{
    __asm__ __volatile__ ("lgdt %0;"::"m"(*gdtr) :"memory");
}

inline
void __ia32eWriteDr0(uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%dr0;"::"r"(val));
}

inline
void __ia32eWriteDr1(uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%dr1;"::"r"(val));
}

inline
void __ia32eWriteDr2(uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%dr2;"::"r"(val));
}

inline
void __ia32eWriteDr3(uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%dr3;"::"r"(val));
}

inline
void __ia32eWriteDr6(uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%dr6;"::"r"(val));
}

inline
void __ia32eWriteDr7(uint64_t val)
{
    __asm__ __volatile__ ("movq %0, %%dr7;"::"r"(val));
}

inline 
uint64_t __ia32eReadDr0(void)
{
    uint64_t val = 0;
    __asm__ __volatile__ ("movq %%dr0, %0;":"=r"(val));
    return val;
}

inline 
uint64_t __ia32eReadDr6(void)
{
    uint64_t val = 0;
    __asm__ __volatile__ ("movq %%dr6, %0;":"=r"(val));
    return val;
}

inline 
uint64_t __ia32eReadDr7(void)
{
    uint64_t val = 0;
    __asm__ __volatile__ ("movq %%dr7, %0;":"=r"(val));
    return val;
}

inline 
void __ia32eInvpcid(uint64_t type, uint32_t pcid, uint64_t addr)
{
    ia32eInvpcidDesc_t desc = {0};

    desc.addr = addr;
    desc.pcid = pcid;

    __asm__ __volatile__ ("invpcid %[d], %[t];"::[d]"m"(desc), [t]"r"(type):"memory");
}

inline 
bool __ia32eVmxon(uintptr_t phys)
{
    uint8_t ret = 0;
    __asm__ __volatile__(
        "vmxon %[phys];"
        "seta %[ret];"
        : [ret] "=r"(ret)
        : [phys] "m"(phys)
        : "cc", "memory");

    return ret;
}

inline 
bool __ia32eVmread(uint64_t field, uint64_t *outp)
{
    uint8_t ret = 0;
    __asm__ __volatile__(
        "vmread %[field], %[outp];"
        "seta %[ret];"
        : [ret] "=r"(ret), [outp] "=rm"(*outp)
        : [field] "r"(field)
        : "cc", "memory");

    return ret;
}

inline 
bool __ia32eVmwrite(uint64_t field, uint64_t inp)
{
    uint8_t ret = 0;
    __asm__ __volatile__(
        "vmwrite %[inp], %[field];"
        "seta %[ret];"
        : [ret] "=r"(ret)
        : [field] "r"(field), [inp] "rm"(inp)
        : "cc", "memory");

    return ret;
}

inline 
bool __ia32eVmread32(uint64_t field, uint32_t *outp)
{
    uint64_t val = 0;
    bool ret = __ia32eVmread(field, &val);
    if (ret)
        *outp = (uint32_t)val;

    return ret;
}

inline 
bool __ia32eVmread16(uint64_t field, uint16_t *outp)
{
    uint64_t val = 0;
    bool ret = __ia32eVmread(field, &val);
    if (ret)
        *outp = (uint16_t)val;

    return ret;
}

inline 
uint64_t ia32eVmread(uint64_t field)
{
    uint64_t val = 0;
    __ia32eVmread(field, &val);
    return val;
}

inline 
uint32_t ia32eVmread32(uint64_t field)
{
    uint32_t val = 0;
    __ia32eVmread32(field, &val);
    return val;
}

inline 
uint16_t ia32eVmread16(uint64_t field)
{
    uint16_t val = 0;
    __ia32eVmread16(field, &val);
    return val;
}

inline 
bool __ia32eVmptrld(uintptr_t phys)
{
    uint8_t ret = 0;
    __asm__ __volatile__(
        "vmptrld %[phys];"
        "seta %[ret];"
        : [ret] "=r"(ret)
        : [phys] "m"(phys)
        : "cc", "memory");

    return ret;
}

inline 
bool __ia32eVmptrst(uint64_t *outp)
{
    uint8_t ret = 0;
    __asm__ __volatile__(
        "vmptrst %[phys];"
        "seta %[ret];"
        : [phys] "=m"(*outp), [ret] "=r"(ret)
        :
        : "cc", "memory");

    return ret;
}

inline 
bool __ia32eVmresume(void)
{
    uint8_t ret = 0;
    __asm__ __volatile__(
        "vmresume;"
        "seta %[ret];"
        : [ret] "=r"(ret)
        :
        : "cc");

    return ret;
}

inline 
bool __ia32eVmclear(uintptr_t phys)
{
    uint8_t ret = 0;
    __asm__ __volatile__(
        "vmclear %[phys];"
        "seta %[ret];"
        : [ret] "=r"(ret)
        : [phys] "m"(phys)
        : "cc", "memory");

    return ret;
}

inline 
bool __ia32eVmlaunch(void)
{
    uint8_t ret = 0;
    __asm__ __volatile__(
        "vmlaunch;"
        "seta %[ret];"
        : [ret] "=r"(ret)
        :
        : "cc");

    return ret;
}

#endif

#endif