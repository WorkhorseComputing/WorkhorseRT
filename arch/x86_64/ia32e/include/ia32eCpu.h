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

#ifndef _IA32E_CPU_H_
#define _IA32E_CPU_H_

#include <generated/autoconf.h>
#include <ia32eAsm.h>

#define IA32E_ASM_SYSCALL_KSP_OFF   0
#define IA32E_ASM_SYSCALL_USP_OFF   8

#define IA32E_ASM_PDE_COUNT         ((CONFIG_X86_64_IA32E_KMAX_SIZE_MB + 1) / 2)

#define IA32E_SPURIOUS_INT_VECTOR   255
#define IA32E_K_EVENT_VECTOR        254
#define IA32E_K_FAKE_ISR_VECTOR     253

#define IA32E_KERNEL_STACK_OF_DR6_MASK          IA32E_DR6_BP0_MASK
#define IA32E_INT_STACK_OF_DR6_MASK             IA32E_DR6_BP1_MASK
#define IA32E_NMI_STACK_OF_DR6_MASK             IA32E_DR6_BP2_MASK
#define IA32E_DOUBLE_FAULT_STACK_OF_DR6_MASK    IA32E_DR6_BP3_MASK

#ifndef ASM_FILE

#include <stdatomic.h>

#include <workhorse/kSched/kSchedTask.h>
#include <export/kCpuInterface.h>
#include <workhorse/kDomainUniverse/kDomainUniverse.h>
#include <lib/dsa/stackq.h>
#include <ia32eVmcs.h>

STATIC_ASSERT((CONFIG_X86_64_IA32E_KSTACK_SIZE % 16) == 0);
STATIC_ASSERT(IA32E_ASM_PDE_COUNT <= 256);

/* lowdata */

extern uint32_t ia32eSignature;
extern uint64_t ia32eMultiboot2Ptr;

extern ia32eIdtDescriptor64_t ia32eIdt64[256];

extern ia32ePde_t ia32ePd[512];
extern ia32ePdpte_t ia32ePdpt[512];
extern ia32ePml4e_t ia32ePml4[512];

extern char __bootStart[];
extern char __bootEnd[];

extern char __kernelStart[];
extern char __kernelEnd[];

extern char __bssStart[];
extern char __bssEnd[];

/* highdata */

extern char ia32eBootStackPadding[];

extern uintptr_t ia32eIsrEntryTable[256];
extern void __ia32eSyscallEntry(void);
extern void __ia32eTaskIdleEntry(void);
extern void __ia32eFakeIsr(uint64_t rsp, uint8_t vector, uint64_t errorCode);

extern char ia32eWakeupBlobStart[];
extern char ia32eWakeupBlobEnd[];

extern uintptr_t ia32eApRsp;
extern uintptr_t ia32eApDr0;

extern char ia32eWakeupBlobSaveArea[];

extern char __bssStart[];
extern char __bssEnd[];

/* Definitions */

struct ia32ePerCpu;
typedef struct ia32ePerCpu ia32ePerCpu_t;

struct ia32eGlobal;
typedef struct ia32eGlobal ia32eGlobal_t;

typedef struct ia32ePerCpu
{
    struct 
    {
        uint64_t syscallKsp;
        uint64_t syscallUsp;
    } syscallStacks ATTR_PACKED;

    ia32eFrame_t *topFrame;
    ia32eFrame_t *currentFrame;
    bool external;

#if CONFIG_X86_64_IA32E_FEATURE_PCID && CONFIG_KMAX_DOMAINS > 4096
    kDomain_t *pcidLastDomain[4096];
#endif

    ia32eRegs_t *syscallRegs;

    ia32ePerCpu_t *thisPtr;
    ia32eGlobal_t *global;

    uint32_t cpuId;
    uint32_t apicId;
    uint32_t acpiUid;

    bool inIsr;
    ia32eStack_t ATTR_ALIGNED(16) intStack;
    ia32eStack_t ATTR_ALIGNED(16) nmiStack;
    ia32eStack_t ATTR_ALIGNED(16) doubleFaultStack;
    ia32eStack_t ATTR_ALIGNED(16) wakeupStack;

    uint32_t apicFrequencyHz;
    uint32_t mxcsrMask;

    kCpuInvokeRoutineFn_t selfIpiFn;

    uint32_t tprCount[14];
    uint8_t tpr;

    uint32_t cpuVersion;
    uint64_t signId;
    union
    {
        uint64_t val;
        struct
        {
            uint64_t enabled : 1;
            uint64_t onlineCapable : 1;
            uint64_t online : 1;
            uint64_t bsp : 1;

            uint64_t monitorMwait : 1;
            uint64_t vme : 1;
            uint64_t de : 1;
            uint64_t pat : 1;
            uint64_t pcid : 1;
            uint64_t fsgsbase : 1;
            uint64_t smep : 1;
            uint64_t smap : 1;
            uint64_t umip : 1;
            uint64_t nx : 1;

            uint64_t rtm : 1;
            uint64_t rtmAbort : 1;

            uint64_t avx10 : 1;

            uint64_t lahf64 : 1;
            uint64_t lzcnt : 1;
            uint64_t prefetchw : 1;
            uint64_t pg1Gb : 1;

            uint64_t l2LineSize : 8;

            uint64_t invTsc : 1;
            uint64_t wbnoinvd : 1;
            uint64_t sgx : 1;

            uint64_t vcpuCapable : 1;
            
            uint64_t vpid : 1;

            uint64_t ept2mb : 1;
            uint64_t ept1gb : 1;
            uint64_t eptUc : 1;
            uint64_t eptAd : 1;

            uint64_t resvd0 : 26;
        } fields;
    } cpuFlags;

    uint32_t extFeaturesSubleafMax;
    uint32_t esigMax;

#if CONFIG_X86_64_IA32E_VTX
    struct 
    {   
        uint32_t revisionId;
        uint32_t ia32eVmxPinbasedCtls;
        uint32_t ia32eVmxProcbasedCtls;
        uint32_t ia32eVmxExitCtls;
        uint32_t ia32eVmxEntryCtls;

        uint64_t hostDr1;
        uint64_t hostDr2;
        uint64_t hostDr3;
        uint64_t hostDr6;
        uint64_t hostDr7;

        struct 
        {
            uint32_t msrAreaCount; 

            /* star/cstar/lstar/fmask, 
               kernelgsbase */

            ia32eVtxMsrEntry_t ATTR_ALIGNED(16) vmexitLoadArea[5];

            /* context switch this !! */

            ia32eVtxMsrEntry_t ATTR_ALIGNED(16) vmexitStoreVmentryLoadArea[5];

            /* switch on enter domain */

            char ATTR_ALIGNED(4096) ioBitmap[8192];

            /* no switch */

            ia32eVtxVmxonRegion_t ATTR_ALIGNED(4096) vmxonRegion;

        } areas;
    } vtx;
#endif

    struct 
    {    
        ia32eTssFull64_t tssFull;

        uint64_t gdtDesc[7];
        ia32eDescriptorReg64_t gdtr;
    } cpuDataStructures;

    struct
    {
        uint32_t threadId;
        uint32_t coreId;
        uint32_t pkgId;
    } topology;

    stackq_t lsrs[IA32E_NUM_VECTORS];

    bool handoffDone;

} ia32ePerCpu_t;

typedef struct ia32eGlobal
{
    union 
    {
        uint32_t val;
        struct 
        {
            uint32_t x2apic : 1;
            uint32_t pcidCapableExists : 1;
            uint32_t vcpuCapableExists : 1;
            uint32_t vpidCapableExists : 1;
            uint32_t allEptAd : 1;
            uint32_t resvd0 : 27;
        } fields;
    } gFlags;

    bool intcSetup;
    ia32ePerCpu_t cpuTable[CONFIG_KMAX_CPUS];
    uint32_t numCpus;
    uint32_t bsp;

    uintptr_t dummyPage;

    struct
    {
        uintptr_t multiboot2Phys;
        uintptr_t multiboot2Ptr;
        size_t multiboot2Size;
    } multiboot2;

    struct 
    {
        uintptr_t rsdtPhys;

        uintptr_t rsdtPtr;
        uintptr_t madtPtr;
        uintptr_t fadtPtr;
        uintptr_t hpetPtr;
    } acpi;

    struct 
    {
        uintptr_t apicMmioPhys;
        uintptr_t apicMmio;
    } apic;

    struct 
    {

        struct
        {
            uintptr_t ioapicMmioPhys;
            uintptr_t ioapicMmio;
            uint32_t ioapicGsiBase;
        } ioapicData[CONFIG_X86_64_IA32E_MAX_IOAPICS];

        uint32_t numIoApics;
    } ioapic;

    struct
    {
        uintptr_t hpetMmio;
        bool found;
    } hpet;

    struct
    {
        uintptr_t acpiPmMmio;
        uint64_t acpiPmPort;
        bool mmio;
        bool found;
    } acpiPm;

    struct
    {
        ia32eIdtDescriptor64_t idt[256];
        ia32eDescriptorReg64_t idtr;
    } cpuDataStructures;

    struct
    {
        kCpuInvokeRoutineFn_t fn;
        volatile atomic_uint_fast32_t rendezvous;
        uint32_t ipiSender;
    } ipiData;
    
    kCpuInvokeRoutineFn_t nmiHandler;
    volatile atomic_uint_fast32_t numCpusOnline;

#if CONFIG_X86_64_IA32E_FEATURE_PCID
    uint32_t pcidCtr;
#endif

#if CONFIG_X86_64_IA32E_VTX

    struct 
    {
#   if CONFIG_X86_64_IA32E_VTX_FEATURE_VPID
        uint32_t vpidCtr;
#   endif

        /* sysenter cs/eip/esp, 
           efer,
           pat,
           gs/fsbase,
           star/lstar/ctsar/fmask, 
           kernelgsbase */

        char ATTR_ALIGNED(4096) msrBitmap[4096];
    } vtxGlobal;

#endif

} ia32eGlobal_t;

STATIC_ASSERT(offsetof(ia32ePerCpu_t, syscallStacks.syscallKsp) == IA32E_ASM_SYSCALL_KSP_OFF);
STATIC_ASSERT(offsetof(ia32ePerCpu_t, syscallStacks.syscallUsp) == IA32E_ASM_SYSCALL_USP_OFF);

STATIC_ASSERT(CONFIG_KMAX_DOMAINS < UINT16_MAX);

#define ia32eInitCpuData(perCpuPtr) __ia32eWrmsr(IA32E_GS_BASE, (uint64_t)(perCpuPtr))
#define ia32eThisCpuData() ((ia32ePerCpu_t *)__ia32eReadgs64(offsetof(ia32ePerCpu_t, thisPtr)))

#define ia32eEarlyKpanic(fmt, ...) do {                 \
    ia32eEarlyPanic("[KPANIC]: " fmt, ##__VA_ARGS__);   \
    UNREACHABLE();                                      \
} while (0)

bool ia32eThisTopology0x1f(uint32_t *lapicId, uint32_t *threadId, uint32_t *coreId, uint32_t *pkgId);
bool ia32eThisTopology0x0b(uint32_t *lapicId, uint32_t *threadId, uint32_t *coreId, uint32_t *pkgId);
void ia32eThisTopologyLegacy(uint32_t *lapicId, uint32_t *threadId, uint32_t *coreId, uint32_t *pkgId);
void ia32eThisTopology(uint32_t *lapicId, uint32_t *threadId, uint32_t *coreId, uint32_t *pkgId);

ia32eGlobal_t *ia32eGetGlobalPtr(void);

uint32_t ia32eMxcsrMask64(void);

void ia32eEarlyIdtInit(void);
void ia32eTssInit(void);
void ia32eGdtInit(void);
void ia32eTrInit(void);
void ia32eIdtInit(void);

void ia32eCpuInit(void);

void ia32eCpuApStart(void);

uint32_t ia32eThisCpuId(void);
void ia32eCpuInvokeAllRendezvous(kCpuInvokeRoutineFn_t fn);
void ia32eCpuSelfIpi(kCpuInvokeRoutineFn_t fn);
void ia32eCpuTaskIdleCtxInit(kSchedTask_t *task);
void ia32eCpuTaskCtxInit(kSchedTask_t *task, uintptr_t pc);
void ia32eCpuTaskSaveCtx(kSchedTask_t *task);
void ia32eCpuTaskRestoreCtx(kSchedTask_t *task);
uintptr_t ia32eCpuSyscallGetReturnAddress(void);
void ia32eCpuSyscallSetReturnAddress(uintptr_t returnAddress);
void ia32eCpuExceptionSetReturnAddress(uintptr_t returnAddress);
void ia32eCpuEnterDomain(kDomain_t *domain);
void ia32eCpuTaskLsrPush(kSchedTask_t *task);
uint32_t ia32eEventSender(void);
bool ia32eCpuIdValidate(uint32_t cpuId);
int ia32eCpuThreadInfoInit(archSchedThreadInfo_t *info, archSchedThreadParam_t *param);
int ia32eCpuLsrInfoInit(archSchedLsrInfo_t *info, archSchedLsrParam_t *param);
int ia32eCpuDomainInfoInit(archDomainInfo_t *info, archDomainParam_t *param);

uint32_t ia32eTimerFrequencyHz(void);
void ia32eTimerArmPeriodic(uint32_t ticks);

void ia32eCallbackActivation(kSchedTask_t *task);
void ia32eCallbackResponse(kSchedTask_t *task);
void ia32eCallbackCpuHandoff(void);

void ia32eIsrPreHandler(ia32eFrame_t *frame);

void ia32eEarlyPanic(const char *str);

void ia32eFlushTlbAll(void);

void __ia32eLogExceptionKernel(ia32eFrame_t *frame);
void ia32eEarlyExceptionHandler(ia32eFrame_t *frame);

void ia32eExceptionHandlerUser(void);
void ia32eExceptionHandlerKernel(void);

void ia32eIsrExternalHandler(void);
void ia32eIsrExceptionHandler(void);

void ia32eIsrEventHandler(void);
void ia32eIsrMain(void);
bool ia32eIsrHandler(ia32eFrame_t *frame);

void ia32eSyscallHandlerStrict(ia32eRegs_t *regs);
void ia32eSyscallHandler(ia32eRegs_t *regs);

#endif

#endif