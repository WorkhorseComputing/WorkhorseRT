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

#ifndef _IA32E_DEFS_H_
#define _IA32E_DEFS_H_

#include <ia32eAsm.h>
#include <lib/dsa/stackq.h>

typedef enum ia32eDomainType
{
    IA32E_DOMAIN_TYPE_NATIVE = 0,
    IA32E_DOMAIN_TYPE_VM =     1
} ia32eDomainType_t;

typedef struct ia32eSchedCtx
{
    ia32eFxsave64_t fpCtx;
    uint64_t dr0;
    uint64_t ksp;
    uint64_t usp;
    ia32eRegs_t *syscallRegs;
    uint64_t userGsbase;
    uint64_t userFsbase;
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
    uint64_t rsp;
    uint64_t rflags;
    uint64_t rip;
    uint16_t cs;
    uint16_t ss;
    ia32eStack_t kStack;

#if CONFIG_IA32E_VTX
    uintptr_t vmcsPhys;
#endif

} ia32eSchedCtx_t;

typedef struct ia32eSchedThreadParam
{
    uint8_t tpr;
    uintptr_t vmcsPhys;
} ia32eSchedThreadParam_t;

typedef struct ia32eSchedThreadInfo
{
    uint8_t tpr;
} ia32eSchedThreadInfo_t;

typedef struct ia32eSchedLsrParam
{
    uint8_t vector;
    uintptr_t vmcsPhys;
} ia32eSchedLsrParam_t;

typedef struct ia32eSchedLsrInfo
{
    uint8_t vector;
} ia32eSchedLsrInfo_t;

typedef struct ia32eDomainInfo
{
    uint64_t cr3;
    uint8_t iopb[8192];
} ia32eDomainInfo_t;

typedef struct ia32eDomainInfoVm
{
    bool vm;
    uintptr_t iobpPhys;

#   if CONFIG_IA32E_VTX_FEATURE_VPID
    uint16_t vpid;
#   endif

} ia32eDomainInfoVm_t;

typedef union ia32eDomainInfoUnion
{
    ia32eDomainInfo_t domainInfo;
    ia32eDomainInfoVm_t domainInfoVm;  
} ia32eDomainInfoUnion_t;

typedef struct ia32eDomainParam
{
    ia32ePml4_t *pml4BaseVirt;
    uint64_t pml4BasePhys;
    uint8_t iopb[8192];
} ia32eDomainParam_t;

typedef struct ia32eDomainParamVm
{
    bool vm;
    uintptr_t iobpPhys;
    uintptr_t eptBasePhys;
} ia32eDomainParamVm_t;

typedef union ia32eDomainParamUnion
{
    ia32eDomainParam_t domainParam;
    ia32eDomainParamVm_t domainParamVm;  
} ia32eDomainParamUnion_t;

typedef struct archSchedCtx
{
    ia32eSchedCtx_t ia32eCtx;
} archSchedCtx_t;

typedef struct archSchedThreadParam
{
    ia32eSchedThreadParam_t ia32eParam;
} archSchedThreadParam_t;

typedef struct archSchedThreadInfo
{
    ia32eSchedThreadInfo_t ia32eInfo;
} archSchedThreadInfo_t;

typedef struct archSchedLsrParam
{
    ia32eSchedLsrParam_t ia32eParam;
} archSchedLsrParam_t;

typedef struct archSchedLsrInfo
{
    ia32eSchedLsrInfo_t ia32eInfo;
} archSchedLsrInfo_t;

typedef struct archDomainInfo
{
    ia32eDomainInfo_t ia32eInfo;
} archDomainInfo_t;

typedef struct archDomainParam
{
    ia32eDomainParam_t ia32eParam;
} archDomainParam_t;

#endif