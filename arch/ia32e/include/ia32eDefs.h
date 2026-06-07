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
#include <ia32eVmcs.h>
#include <lib/dsa/stackq.h>
#include <lib/dsa/dq.h>
#include <lib/dsa/bitmap.h>
#include <lib/mcsLock.h>

typedef struct ia32eVtxParam
{
    ia32eVtxVmcsRegion_t *vmcsVirt;
    uintptr_t vmcsPhys;
} ia32eVtxParam_t;

typedef struct ia32eVtxTaskInfo 
{
    ia32eVtxParam_t vtxParam;
    uint8_t vcpuId;
    dqListNode_t vcpuVectorNode;
} ia32eVtxTaskInfo_t;

typedef struct ia32eVtxVectoredEvent
{
    union 
    {
        uint32_t val;
        struct
        {
            uint32_t vector : 8;
            uint32_t type : 3;
            uint32_t deliverErrcode : 1;
            uint32_t advance : 1;
            uint32_t valid : 1;
            uint32_t mode : 3;
            uint32_t resvd0 : 15;
        } fields;
    } delivery;
    uint64_t errcode;
} ia32eVtxVectoredEvent_t;

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
    struct 
    { 
        uint64_t vmexitStoreVmentryLoadAreaData[5];

        ia32eVtxVectoredEvent_t lostEvent;
        ia32eVtxVectoredEvent_t syntheticEvent;

        struct
        {
            mcsLock_t lock;

            union
            {
                uint32_t val;
                struct
                {
                    uint32_t vmcsInitialized : 1;
                    uint32_t apicBaseBsp : 1;
                    uint32_t apicBaseGlobalEn : 1;
                    uint32_t waitForSipi : 1;
                    uint32_t tpr : 4;
                    uint32_t reserved0 : 24;
                } fields;
            } local;

            union
            {
                uint8_t val;
                struct
                {
                    uint8_t initPending : 1;
                    uint8_t sipiPending : 1;
                    uint8_t nmiPending : 1;
                    uint8_t reserved0 : 5;
                } fields;
            } latch;

            uint32_t sivr;

            uint32_t isr[8];
            uint32_t tmr[8];
            uint32_t irr[8];

            uint32_t esr;

            uint64_t icr;

            uint32_t lvtTImer;
            
            uint32_t initCount;
            uint32_t divConf;
            uint32_t selfIpi;
        } x2apic;
    } vtx;
#endif

} ia32eSchedCtx_t;

typedef struct ia32eSchedThreadParam
{
    uint8_t tpr;

#if CONFIG_IA32E_VTX
    ia32eVtxParam_t vtxParam;
#endif

} ia32eSchedThreadParam_t;

typedef struct ia32eSchedThreadInfo
{
    uint8_t tpr;

#if CONFIG_IA32E_VTX
    ia32eVtxTaskInfo_t vtxInfo;
#endif

} ia32eSchedThreadInfo_t;

typedef struct ia32eSchedLsrParam
{
    uint8_t vector;
    
#if CONFIG_IA32E_VTX
    ia32eVtxParam_t vtxParam;
#endif

} ia32eSchedLsrParam_t;

typedef struct ia32eSchedLsrInfo
{
    uint8_t vector;

#if CONFIG_IA32E_VTX
    ia32eVtxTaskInfo_t vtxInfo;
#endif

} ia32eSchedLsrInfo_t;

typedef struct ia32eDomainInfo
{
    uint64_t cr3;
    uint8_t iopb[8192];

#if CONFIG_IA32E_VTX
    bool vm;

#   if CONFIG_IA32E_VTX_FEATURE_VPID
    uint16_t vpid;
#   endif

    uint8_t numVcpus;
    dq_t vcpuVector;
#endif

} ia32eDomainInfo_t;

typedef struct ia32eDomainParam
{
    ia32ePml4_t *pml4BaseVirt;
    uintptr_t pml4BasePhys;
    uint8_t iopb[8192];

#if CONFIG_IA32E_VTX
    bool vm;
#endif

} ia32eDomainParam_t;

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