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

#include <ia32eCpu.h>
#include <ia32eApic.h>
#include <ia32eCpuVtx.h>
#include <export/kDbgInterface.h>
#include <import/kTickHandler.h>
#include <import/kSyscallHandler.h>
#include <import/kLsrHandler.h>
#include <import/kExceptionHandler.h>
#include <workhorse/kTick/kTick.h>
#include <lib/mcsLock.h>
#include <errno.h>

static 
ia32eGlobal_t ia32eGlobal = {0};

static 
mcsLock_t exceptionLock = INITIALIZE_MCSLOCK();

static 
char *vectorToStr[] = 
{
    [IA32E_DIVIDE_ERROR]                    = "DIVIDE_ERROR",
    [IA32E_DEBUG_EXCEPTION]                 = "DEBUG_EXCEPTION",
    [IA32E_NMI]                             = "NMI",
    [IA32E_BREAKPOINT]                      = "BREAKPOINT",
    [IA32E_OVERFLOW]                        = "OVERFLOW",
    [IA32E_BOUND_RANGE_EXCEEDED]            = "BOUND_RANGE_EXCEEDED", 
    [IA32E_INVALID_OPCODE]                  = "INVALID_OPCODE",
    [IA32E_DEVICE_NOT_AVAILABLE]            = "DEVICE_NOT_AVAILABLE",
    [IA32E_DOUBLE_FAULT]                    = "DOUBLE_FAULT",
    [IA32E_COPROCESSOR_SEGMENT_OVERRUN]     = "COPROCESSOR_SEGMENT_OVERRUN",
    [IA32E_INVALID_TSS]                     = "INVALID_TSS",
    [IA32E_SEGMENT_NOT_PRESENT]             = "SEGMENT_NOT_PRESENT",
    [IA32E_STACK_SEGMENT_FAULT]             = "STACK_SEGMENT_FAULT",
    [IA32E_GENERAL_PROTECTION_FAULT]        = "GENERAL_PROTECTION_FAULT",
    [IA32E_PAGE_FAULT]                      = "PAGE_FAULT",
    [IA32E_VECTOR15]                        = "VECTOR15",
    [IA32E_MATH_FAULT]                      = "MATH_FAULT",
    [IA32E_ALIGNMENT_CHECK]                 = "ALIGNMENT_CHECK",
    [IA32E_MACHINE_CHECK]                   = "MACHINE_CHECK",
    [IA32E_SIMD_FLOATING_POINT_EXCEPTION]   = "SIMD_FLOATING_POINT_EXCEPTION",
    [IA32E_VIRTUALISATION_EXCEPTION]        = "VIRTUALISATION_EXCEPTION",
    [IA32E_CONTROL_PROTECTION_EXCEPTION]    = "CONTROL_PROTECTION_EXCEPTION",
    [IA32E_VECTOR22]                        = "VECTOR22",
    [IA32E_VECTOR23]                        = "VECTOR23",
    [IA32E_VECTOR24]                        = "VECTOR24",
    [IA32E_VECTOR25]                        = "VECTOR25",
    [IA32E_VECTOR26]                        = "VECTOR26",
    [IA32E_VECTOR27]                        = "VECTOR27",
    [IA32E_VECTOR28]                        = "VECTOR28",
    [IA32E_VECTOR29]                        = "VECTOR29",
    [IA32E_VECTOR30]                        = "VECTOR30",
    [IA32E_VECTOR31]                        = "VECTOR31"
};

static
ia32eInterruptType_t interruptTypes[] =
{
    [IA32E_DIVIDE_ERROR]                    = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_DEBUG_EXCEPTION]                 = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_NMI]                             = IA32E_INTERRUPT_TYPE_NMI,
    [IA32E_BREAKPOINT]                      = IA32E_INTERRUPT_TYPE_SOFTWARE_EXCEPTION,
    [IA32E_OVERFLOW]                        = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_BOUND_RANGE_EXCEEDED]            = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_INVALID_OPCODE]                  = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_DEVICE_NOT_AVAILABLE]            = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_DOUBLE_FAULT]                    = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_COPROCESSOR_SEGMENT_OVERRUN]     = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_INVALID_TSS]                     = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_SEGMENT_NOT_PRESENT]             = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_STACK_SEGMENT_FAULT]             = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_GENERAL_PROTECTION_FAULT]        = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_PAGE_FAULT]                      = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_VECTOR15]                        = IA32E_INTERRUPT_TYPE_EXTERNAL,
    [IA32E_MATH_FAULT]                      = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_ALIGNMENT_CHECK]                 = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_MACHINE_CHECK]                   = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_SIMD_FLOATING_POINT_EXCEPTION]   = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_VIRTUALISATION_EXCEPTION]        = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_CONTROL_PROTECTION_EXCEPTION]    = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION,
    [IA32E_VECTOR22]                        = IA32E_INTERRUPT_TYPE_EXTERNAL,
    [IA32E_VECTOR23]                        = IA32E_INTERRUPT_TYPE_EXTERNAL,
    [IA32E_VECTOR24]                        = IA32E_INTERRUPT_TYPE_EXTERNAL,
    [IA32E_VECTOR25]                        = IA32E_INTERRUPT_TYPE_EXTERNAL,
    [IA32E_VECTOR26]                        = IA32E_INTERRUPT_TYPE_EXTERNAL,
    [IA32E_VECTOR27]                        = IA32E_INTERRUPT_TYPE_EXTERNAL,
    [IA32E_VECTOR28]                        = IA32E_INTERRUPT_TYPE_EXTERNAL,
    [IA32E_VECTOR29]                        = IA32E_INTERRUPT_TYPE_EXTERNAL,
    [IA32E_VECTOR30]                        = IA32E_INTERRUPT_TYPE_EXTERNAL,
    [IA32E_VECTOR31]                        = IA32E_INTERRUPT_TYPE_EXTERNAL
};

static
void ia32eFlushTlb(void)
{
    __ia32eCr4ReenablePge();
}

static
inline 
void ia32eReloadTpr(void)
{
    ia32ePerCpu_t *cpu = NULL;
    int32_t i = 0;
    uint32_t newTpr = 0;

    cpu = ia32eThisCpuData();
    newTpr = 1;

    for (i = ARRAY_LEN(cpu->tprCount) - 1; i >= 0; i--) {
        
        if (cpu->tprCount[i] > 0) {
            newTpr = i + 1;
            break;
        }
    }

    __ia32eWriteCr8(newTpr);
    cpu->tpr = newTpr;
}

bool ia32eThisTopology0x1f(uint32_t *lapicId, uint32_t *threadId, uint32_t *coreId, uint32_t *pkgId)
{
    uint32_t regs[4] = {0};

    uint32_t fullLapicId = 0;
    uint32_t smtShiftWidth = 0;
    uint32_t coreShiftWidth = 0;

    uint32_t i = 0;

    uint32_t levelType = 0;
    uint32_t shift = 0;

    uint32_t coreMask = 0;
    
    __ia32eCpuid(&regs[0], &regs[1], &regs[2], &regs[3]);
    if (regs[0] < 0x1f)
        return false;

    ia32eCpuid(0x1f, 0, &regs[0], &regs[1], &regs[2], &regs[3]);
    if (regs[1] == 0)
        return false;

    /* edx holds x2apic id */
    fullLapicId = regs[3];
    smtShiftWidth = 0;
    coreShiftWidth = 0;

    for (i = 0; ; i++) {
        ia32eCpuid(0x1f, i, &regs[0], &regs[1], &regs[2], &regs[3]);
        
        levelType = (regs[2] >> 8) & 0xff;
        shift = regs[0] & 0x1f;

        if (levelType == IA32E_INVAL) 
            break;

        switch (levelType) {

            case IA32E_SMT:
                smtShiftWidth = shift;
                break;

            case IA32E_CORE:
                coreShiftWidth = shift;
                break;

            default:
                break;
        }
    }

    *threadId = smtShiftWidth != 0 ? fullLapicId & ((1U << smtShiftWidth) - 1) : 0;
  
    if (coreShiftWidth != 0) {
        coreMask = (1U << coreShiftWidth) - 1;
        *coreId = (fullLapicId & coreMask) >> smtShiftWidth;
        *pkgId = fullLapicId >> coreShiftWidth;
    } else {

        /* handle weird single core per pkg topos */
        *coreId = 0;
        *pkgId = fullLapicId >> smtShiftWidth;
    }

    *lapicId = fullLapicId;
    return true;
}

bool ia32eThisTopology0x0b(uint32_t *lapicId, uint32_t *threadId, uint32_t *coreId, uint32_t *pkgId)
{
    uint32_t regs[4] = {0};

    uint32_t tmpLapicId = 0;
   
    uint32_t subleaf0Type = 0;
    uint32_t subleaf0Shift = 0;

    uint32_t subleaf1Type = 0;
    uint32_t subleaf1Shift = 0;

    uint32_t threadIdShiftWidth = 0;
    uint32_t coreIdShiftWidth = 0;

    uint32_t threadIdMask = 0;
    uint32_t coreIdMask = 0;

    /* again, check if its supported and doesnt return nothing */
    __ia32eCpuid(&regs[0], &regs[1], &regs[2], &regs[3]);
    if (regs[0] < 0x0b)
        return false;

    ia32eCpuid(0x0b, 0, &regs[0], &regs[1], &regs[2], &regs[3]);
    if (regs[1] == 0)
        return false;

    /* edx will hold the lapic id */
    tmpLapicId = regs[3];

    /* ecx[15:8] will hold the type 0 - inval, 1 - smt, 2 - core 
       eax[4:0] will hold the shift */
    subleaf0Type = (regs[2] >> 8) & 0xff;
    subleaf0Shift = regs[0] & 0x1f;

    ia32eCpuid(0x0b, 1, &regs[0], &regs[1], &regs[2], &regs[3]);
    subleaf1Type = (regs[2] >> 8) & 0xff;
    subleaf1Shift = regs[0] & 0x1f;

    if (subleaf0Type == IA32E_SMT) {

        threadIdShiftWidth = subleaf0Shift;

        if (subleaf1Type == IA32E_CORE && subleaf1Shift != 0)
            coreIdShiftWidth = subleaf1Shift;

    } else if (subleaf0Type == IA32E_CORE) {

        coreIdShiftWidth = subleaf0Shift;
        if (subleaf1Type == IA32E_SMT && subleaf1Shift != 0)
            threadIdShiftWidth = subleaf1Shift; 

    } else {
        return false;
    }

    /* check if were to extract the thread id, which we should if smt is on */
    if (threadIdShiftWidth != 0) {
        threadIdMask = (1U << threadIdShiftWidth) - 1;
        *threadId = tmpLapicId & threadIdMask;
    } else {
        *threadId = 0;
    }

    /* check if were multi physical core */
    if (coreIdShiftWidth != 0) {

        /* normal regular shmegular smt shit */
        if (coreIdShiftWidth > threadIdShiftWidth) {

            coreIdMask = (1U << coreIdShiftWidth) - 1;
            *coreId = (tmpLapicId & coreIdMask) >> threadIdShiftWidth;
            *pkgId = tmpLapicId >> coreIdShiftWidth;

        /* weird fucking edge case if its a single core with smt enabled */
        } else if (coreIdShiftWidth == threadIdShiftWidth) {

            *coreId = 0;
            *pkgId = tmpLapicId >> threadIdShiftWidth;

        /* shouldnt happen with how shits supposed to be layed out */
        } else {
            return false;
        }

    } else {

        /* single core i guess?? */
        *coreId = tmpLapicId;
        *pkgId = 0;
    }

    *lapicId = tmpLapicId;
    return true;
}

void ia32eThisTopologyLegacy(uint32_t *lapicId, uint32_t *threadId, uint32_t *coreId, uint32_t *pkgId)
{
    uint32_t regs[4] = {0};
    uint32_t threadCount = 0;
    bool smtSupported = false;
    uint32_t coresPerPkg = 0;
    uint32_t threadsPerCore = 0;
    uint32_t threadIdBits = 0;
    uint32_t coreIdBits = 0;
    uint32_t threadIdMask = 0;
    uint32_t coreIdMask = 0;

    ia32eCpuid(1, 0, &regs[0], &regs[1], &regs[2], &regs[3]);

    /* ebx[31:24] will hold the lapic id for us */
    *lapicId = (regs[1] >> 24) & 0xff;

    /* ebx[23:16] will tell us the lp count */
    threadCount = (regs[1] >> 16) & 0xff;

    /* check edx[28] to see if smt is supported */
    smtSupported = (regs[3] >> 28) & 1;
    
    coresPerPkg = 1;

    /* we can get the num cores from leaf 4 */
    ia32eCpuid(0, 0, &regs[0], &regs[1], &regs[2], &regs[3]);
    if (regs[0] >= 4) {
        ia32eCpuid(4, 0, &regs[0], &regs[1], &regs[2], &regs[3]);
        
        /* gotta make sure the cache heirarchy it passes is
           legit so we know its actually supported,
           eax[31:26] will hold the physical core count - 1 */
        if ((regs[0] & 0x1f) != 0)
            coresPerPkg = ((regs[0] >> 26) & 0x3f) + 1;
    }

    /* gotta check if smt isnt on */
    if (!smtSupported || threadCount < 2) {
        *threadId = 0;
        *coreId = *lapicId;
        *pkgId = 0;
        return;
    }

    /* gotta create bitmasks now so we can derive id's from the lapic id */
    threadsPerCore = threadCount / coresPerPkg;

    while (threadsPerCore > 1) {
        threadIdBits++;
        threadsPerCore >>= 1;
    }

    while (coresPerPkg > 1) {
        coreIdBits++;
        coresPerPkg >>= 1;
    }

    threadIdMask = (1U << threadIdBits) - 1;
    coreIdMask = (1U << (threadIdBits + coreIdBits)) - 1;

    /* now we have our masks we can derive the id's, basically 
       it'll be layed out like [pkgId | coreId | threadId] */
    *threadId = *lapicId & threadIdMask;
    *coreId = (*lapicId & coreIdMask) >> threadIdBits;
    *pkgId = *lapicId >> (coreIdBits + threadIdBits);
}


void ia32eThisTopology(uint32_t *lapicId, uint32_t *threadId, uint32_t *coreId, uint32_t *pkgId)
{
    /* grab the core topology with the first leaf that is supported by 
       preference, otherwise it can lead to enumerating wrong info */
    if (ia32eThisTopology0x1f(lapicId, threadId, coreId, pkgId))
        return;

    if (ia32eThisTopology0x0b(lapicId, threadId, coreId, pkgId))
        return;

    ia32eThisTopologyLegacy(lapicId, threadId, coreId, pkgId);
}

void ia32eCpuInit(void)
{
    ia32ePerCpu_t *cpu = NULL;
    uint64_t cr4 = 0;
    uint32_t regs1[4] = {0};
    uint32_t regs7[4] = {0};
    uint32_t regsEsig0[4] = {0};
    uint32_t regsEsig1[4] = {0};
    uint32_t regsEsig7[4] = {0};
    uint32_t regs18[4] = {0};
    uint64_t efer = 0;
    uint64_t dr7 = 0;

    cpu = ia32eThisCpuData();
    cr4 = __ia32eReadCr4();
    efer = __ia32eRdmsr(IA32E_EFER);

    ia32eCpuid(1, 0, &regs1[0], &regs1[1], &regs1[2], &regs1[3]);

    cpu->cpuVersion = regs1[0];

    cpu->cpuFlags.fields.monitorMwait = (regs1[2] & IA32E_CPUID1_C_MONITOR_MWAIT_MASK) != 0;
    cpu->cpuFlags.fields.vme = (regs1[3] & IA32E_CPUID1_D_VME_MASK) != 0;

    if ((regs1[3] & IA32E_CPUID1_D_DE_MASK) != 0) {
        cr4 |= IA32E_CR4_DE_MASK;
        cpu->cpuFlags.fields.de = 1;
    }

    cpu->cpuFlags.fields.pat = (regs1[3] & IA32E_CPUID1_D_PAT_MASK) != 0;

    ia32eCpuid(7, 0, &regs7[0], &regs7[1], &regs7[2], &regs7[3]);

#if CONFIG_IA32E_FEATURE_PCID

    if ((regs1[2] & IA32E_CPUID1_C_PCID_MASK) != 0 && (regs7[1] & IA32E_CPUID7_0_B_INVPCID_MASK) != 0) {
        cr4 |= IA32E_CR4_PCIDE_MASK;
        cpu->cpuFlags.fields.pcid = 1;
        cpu->global->gFlags.fields.pcidCapableExists = 1;
    }

#endif

    if ((regs7[1] & IA32E_CPUID7_0_B_FSGSBASE) != 0) {
        cr4 |= IA32E_CR4_FSGSBASE_MASK;
        cpu->cpuFlags.fields.fsgsbase = 1;
    }

    if ((regs7[1] & IA32E_CPUID7_0_B_SMEP_MASK) != 0) {

#if CONFIG_IA32E_FEATURE_SMEP
        cr4 |= IA32E_CR4_SMEP_MASK;
#endif

        cpu->cpuFlags.fields.smep = 1;
    }

    if ((regs7[1] & IA32E_CPUID7_0_B_SMAP_MASK) != 0) {

#if CONFIG_IA32E_FEATURE_SMAP
        cr4 |= IA32E_CR4_SMAP_MASK;
#endif

        cpu->cpuFlags.fields.smap = 1;
    }

#if CONFIG_IA32E_FEATURE_UMIP

    if ((regs7[2] & IA32E_CPUID7_0_C_UMIP_MASK) != 0) {
        cr4 |= IA32E_CR4_UMIP_MASK;
        cpu->cpuFlags.fields.umip = 1;
    }

#endif

    ia32eCpuid(IA32E_CPUID_ESIG0, 0, &regsEsig0[0], &regsEsig0[1], &regsEsig0[2], &regsEsig0[3]);
    
    cpu->esigMax = regsEsig0[0];

    if (regsEsig0[0] >= IA32E_CPUID_ESIG1) {

        ia32eCpuid(IA32E_CPUID_ESIG1, 0, &regsEsig1[0], &regsEsig1[1], &regsEsig1[2], &regsEsig1[3]);
        if ((regsEsig1[3] & IA32E_CPUID_ESIG1_D_NX_MASK) != 0) {
            efer |= IA32E_EFER_XD_ENABLE_MASK;
            cpu->cpuFlags.fields.nx = 1;
        }
    }

    if (regsEsig0[0] >= IA32E_CPUID_ESIG7) {
        ia32eCpuid(IA32E_CPUID_ESIG7, 0, &regsEsig7[0], &regsEsig7[1], &regsEsig7[2], &regsEsig7[3]);
        cpu->cpuFlags.fields.invTsc = (regsEsig7[3] & IA32E_CPUID_ESIG7_D_INVARIANT_TSC_MASK) != 0;
    }

    ia32eCpuid(18, 0, &regs18[0], &regs18[1], &regs18[2], &regs18[3]);
    cpu->cpuFlags.fields.sgx = (regs18[0] & IA32E_CPUID18_0_A_SGX1_MASK) != 0;

#if CONFIG_IA32E_TSD
    cr4 |= IA32E_CR4_TSD_MASK;
#endif

    efer |= IA32E_EFER_SYSCALL_ENABLE_MASK;
    __ia32eWrmsr(IA32E_STAR, ((((IA32E_UDS_IDX - 1) << 3) | 3) << 48) | (IA32E_KCS_SELECTOR << 32));
    __ia32eWrmsr(IA32E_LSTAR, (uintptr_t)__ia32eSyscallEntry);
    __ia32eWrmsr(IA32E_FMASK, IA32E_FLAGS_TF_MASK | IA32E_FLAGS_IF_MASK | IA32E_FLAGS_DF_MASK | IA32E_FLAGS_AC_MASK);

    __ia32eWriteCr4(cr4);
    __ia32eWrmsr(IA32E_EFER, efer);

    __ia32eWriteDr1((uintptr_t)cpu->intStack.padding);
    __ia32eWriteDr2((uintptr_t)cpu->nmiStack.padding);
    __ia32eWriteDr3((uintptr_t)cpu->doubleFaultStack.padding);

    barrier();

    dr7 = __ia32eReadDr7();
    dr7 |= IA32E_DR7_BP1_RW_8B_MASK; 
    dr7 |= IA32E_DR7_BP2_RW_8B_MASK; 
    dr7 |= IA32E_DR7_BP3_RW_8B_MASK;
    __ia32eWriteDr7(dr7);

#if CONFIG_IA32E_VTX

    if ((regs1[2] & IA32E_CPUID1_C_VMX_MASK) != 0)
        ia32eCpuVtxInit();

#endif
}

ia32eGlobal_t *ia32eGetGlobalPtr(void)
{
    return &ia32eGlobal;
}

uint32_t ia32eMxcsrMask64(void)
{
    ia32eFxsave64_t ATTR_ALIGNED(16) fxsave = {0};
    __ia32eFxsave(&fxsave);
    return fxsave.mxcsrMask;
}

void ia32eEarlyIdtInit(void)
{
    ia32eGlobal_t *global = NULL;
    ia32eIdtDescriptor64_t *ia32eIdt64High = NULL;
    uint32_t i = 0;

    global = ia32eGetGlobalPtr();
    ia32eIdt64High = global->cpuDataStructures.idt;
    
    for (i = 0; i < ARRAY_LEN(ia32eIsrEntryTable); i++) {

        ia32eIdt64High[i].ist = i == IA32E_NMI ? 2 : i == IA32E_DOUBLE_FAULT ? 3 : 1;
        ia32eIdt64High[i].attr = (1 << 7) | IA32E_INTERRUPT_GATE64;
        ia32eIdt64High[i].selector = IA32E_KCS_SELECTOR;
        ia32eIdt64High[i].offset_low = ia32eIsrEntryTable[i] & 0xffff;
        ia32eIdt64High[i].offset_mid = (ia32eIsrEntryTable[i] >> 16) & 0xffff;
        ia32eIdt64High[i].offset_high = ia32eIsrEntryTable[i] >> 32;
        ia32eIdt64High[i].reserved0 = 0;

        ia32eIdt64[i].ist = 0;
        ia32eIdt64[i].attr = (1 << 7) | IA32E_INTERRUPT_GATE64;
        ia32eIdt64[i].selector = IA32E_KCS_SELECTOR;
        ia32eIdt64[i].offset_low = ia32eIsrEntryTable[i] & 0xffff;
        ia32eIdt64[i].offset_mid = (ia32eIsrEntryTable[i] >> 16) & 0xffff;
        ia32eIdt64[i].offset_high = ia32eIsrEntryTable[i] >> 32;
        ia32eIdt64[i].reserved0 = 0;
    }   

    ia32eIdt64High[IA32E_BREAKPOINT].attr |= (3 << 5);

    global->cpuDataStructures.idtr.base = (uintptr_t)ia32eIdt64High;
    global->cpuDataStructures.idtr.limit = sizeof(global->cpuDataStructures.idt) - 1;
}

void ia32eTssInit(void)
{
    ia32ePerCpu_t *cpu = NULL;

    cpu = ia32eThisCpuData();

    cpu->cpuDataStructures.tssFull.tss.rsp0 = (uintptr_t)(&cpu->intStack.stack[sizeof(cpu->intStack.stack)]);
    cpu->cpuDataStructures.tssFull.tss.ist1 = (uintptr_t)(&cpu->intStack.stack[sizeof(cpu->intStack.stack)]);
    cpu->cpuDataStructures.tssFull.tss.ist2 = (uintptr_t)(&cpu->intStack.stack[sizeof(cpu->nmiStack.stack)]);
    cpu->cpuDataStructures.tssFull.tss.ist3 = (uintptr_t)(&cpu->intStack.stack[sizeof(cpu->doubleFaultStack.stack)]);

    cpu->cpuDataStructures.tssFull.tss.iopbBase = offsetof(ia32eTssFull64_t, iopb);
    cpu->cpuDataStructures.tssFull.iopb[ARRAY_LEN(cpu->cpuDataStructures.tssFull.iopb) - 1] = 0xff;
}

void ia32eGdtInit(void)
{
    ia32ePerCpu_t *cpu = NULL;
    uint64_t *gdt = NULL;
    uintptr_t tssBase = 0;
    uint64_t tssLimit = 0;

    cpu = ia32eThisCpuData();
    gdt = cpu->cpuDataStructures.gdtDesc;

    tssBase = (uintptr_t)&cpu->cpuDataStructures.tssFull;
    tssLimit = sizeof(ia32eTssFull64_t) - 1;

    gdt[IA32E_KCS_IDX] = IA32E_ASM_KCS_DESC;
    gdt[IA32E_KDS_IDX] = IA32E_ASM_KDS_DESC;
    gdt[IA32E_UCS_IDX] = IA32E_ASM_UCS_DESC;
    gdt[IA32E_UDS_IDX] = IA32E_ASM_UDS_DESC;

    gdt[IA32E_TR_LOW_IDX] = (tssLimit & 0xffff) | ((tssBase & 0xffff) << 16) | 
                            (((tssBase >> 16) & 0xff) << 32) | (0x9ULL << 40) | (1ULL << 47) | 
                            (((tssLimit >> 16) & 0xf) << 48) | (((tssBase >> 24) & 0xff) << 56);

    gdt[IA32E_TR_HIGH_IDX] = tssBase >> 32;

    cpu->cpuDataStructures.gdtr.limit = sizeof(cpu->cpuDataStructures.gdtDesc) - 1;
    cpu->cpuDataStructures.gdtr.base = (uintptr_t)gdt;

    __ia32eLgdt(&cpu->cpuDataStructures.gdtr);
}

void ia32eTrInit(void)
{
    __ia32eLtr(IA32E_TR_SELECTOR);
}

void ia32eIdtInit(void)
{
    __ia32eLidt(&ia32eThisCpuData()->global->cpuDataStructures.idtr);
}

ATTR_NORETURN
void ia32eCpuApStart(void)
{
    ia32eGlobal_t *global = NULL;
    uint32_t numCpus = 0;

    uint32_t lapicId = 0;
    uint32_t threadId = 0;
    uint32_t coreId = 0;
    uint32_t pkgId = 0;

    ia32ePerCpu_t *cpu = NULL;
    uint32_t i = 0;

    global = ia32eGetGlobalPtr();
    numCpus = global->numCpus;

    ia32eThisTopology(&lapicId, &threadId, &coreId, &pkgId);

    for (i = 0; i < numCpus; i++) {
        
        if (global->cpuTable[i].apicId == lapicId) {
            cpu = &global->cpuTable[i];
            break;
        }
    }

    __ia32eWrmsr(IA32E_GS_BASE, (uintptr_t)cpu);

    cpu->topology.threadId = threadId;
    cpu->topology.coreId = coreId;
    cpu->topology.pkgId = pkgId;

    cpu->cpuFlags.fields.online = 1;

    ia32eApApicSync();

#if CONFIG_IA32E_APPLY_MADT_NMI_OVERRIDES
    
    ia32eApicConfigMadtNmiOverrides();

#endif

    ia32eTssInit();
    ia32eGdtInit();
    ia32eTrInit();
    ia32eIdtInit();

    ia32eApicEnable(IA32E_SPURIOUS_INT_VECTOR);
    cpu->apicFrequencyHz = ia32eApicFrequencyHz(IA32E_SPURIOUS_INT_VECTOR);
    cpu->mxcsrMask = ia32eMxcsrMask64();
    ia32eCpuInit();

    cpuEnableInterrupts();
    atomic_fetch_add(&global->numCpusOnline, 1);

    __ia32eHltForever();
    
    UNREACHABLE();
}

/* Cpu */

uint32_t ia32eThisCpuId(void)
{
    return ia32eThisCpuData()->cpuId;
}

void ia32eCpuInvokeAllRendezvous(kCpuInvokeRoutineFn_t fn)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eGlobal_t *global = NULL;
    uint32_t cpuId = 0;

    uint32_t sentCount = 0;
    uint32_t numCpus = 0;
    uint32_t i = 0;
    ia32ePerCpu_t *targetCpu = NULL;

    cpu = ia32eThisCpuData();
    global = cpu->global;
    cpuId = cpu->cpuId;

    atomic_store(&global->ipiData.rendezvous, 0);
    global->ipiData.fn = fn;

    sentCount = 0;
    numCpus = global->numCpus;
    
    K_DYNAMIC_ASSERT(numCpus <= ARRAY_LEN(global->cpuTable));

    for (i = 0; i < numCpus; i++) {

        targetCpu = &global->cpuTable[i];

        if (targetCpu->cpuFlags.fields.online == 0 || targetCpu->cpuId == cpuId)
            continue;

        ia32eApicSendIpi(targetCpu->apicId, IA32E_K_EVENT_VECTOR, IA32E_DM_NORMAL, 
                         IA32E_XAPIC_DEST_PHYSICAL, IA32E_XAPIC_SINGLE_TARGET);
        sentCount++;
    }

    if (fn)
        fn();

    spinUntil(atomic_load(&global->ipiData.rendezvous) == sentCount);
    global->ipiData.fn = NULL;
}

void ia32eCpuSelfIpi(kCpuInvokeRoutineFn_t fn)
{
    ia32ePerCpu_t *cpu = NULL;
    uint64_t rsp = 0;
    uintptr_t status = 0;

    cpu = ia32eThisCpuData();
    rsp = (uint64_t)&cpu->intStack.stack[sizeof(cpu->intStack.stack)];
    status = cpuReadStatus();

    cpuDisableInterrupts();

    cpu->selfIpiFn = fn;
    __ia32eFakeIsr(rsp, IA32E_K_FAKE_ISR_VECTOR, 0);

    cpuWriteStatus(status);
}

void ia32eCpuTaskIdleCtxInit(kSchedTask_t *task)
{
    task->ctx.ia32eCtx.fpCtx.fcw = IA32E_DEFAULT_FCW;
    task->ctx.ia32eCtx.fpCtx.mxcsr = IA32E_DEFAULT_MXCSR;
    task->ctx.ia32eCtx.fpCtx.mxcsrMask = ia32eThisCpuData()->mxcsrMask;

    task->ctx.ia32eCtx.rflags |= IA32E_FLAGS_ALWAYS1_MASK;
    task->ctx.ia32eCtx.rflags |= IA32E_FLAGS_IF_MASK;

    task->ctx.ia32eCtx.rip = (uintptr_t)__ia32eTaskIdleEntry;

    task->ctx.ia32eCtx.cs = IA32E_KCS_SELECTOR;
    task->ctx.ia32eCtx.ss = IA32E_KDS_SELECTOR;
}

void ia32eCpuTaskCtxInit(kSchedTask_t *task, uintptr_t pc)
{
    memset(&task->ctx, 0, sizeof(task->ctx));
    
#if CONFIG_IA32E_VTX

    if (task->domain.curDomain->archInfo.ia32eInfo.vm) {
        ia32eCpuVtxTaskInit(task);
        return;
    }

#endif

    task->ctx.ia32eCtx.fpCtx.fcw = IA32E_DEFAULT_FCW;
    task->ctx.ia32eCtx.fpCtx.mxcsr = IA32E_DEFAULT_MXCSR;
    task->ctx.ia32eCtx.fpCtx.mxcsrMask = ia32eThisCpuData()->mxcsrMask;

    task->ctx.ia32eCtx.rflags |= IA32E_FLAGS_ALWAYS1_MASK;
    task->ctx.ia32eCtx.rflags |= IA32E_FLAGS_IF_MASK;

    task->ctx.ia32eCtx.rip = pc;

    task->ctx.ia32eCtx.cs = IA32E_UCS_SELECTOR;
    task->ctx.ia32eCtx.ss = IA32E_UDS_SELECTOR;

    task->ctx.ia32eCtx.dr0 = (uintptr_t)task->ctx.ia32eCtx.kStack.padding;
    task->ctx.ia32eCtx.ksp = (uint64_t)&task->ctx.ia32eCtx.kStack.stack[sizeof(task->ctx.ia32eCtx.kStack.stack)];
}

void ia32eCpuTaskSaveCtx(kSchedTask_t *task)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eFrame_t *topFrame = NULL;

    cpu = ia32eThisCpuData();
    topFrame = cpu->topFrame;

    task->ctx.ia32eCtx.fpCtx = topFrame->regs.fxsaveRegion;

    task->ctx.ia32eCtx.dr0 = __ia32eReadDr0();

    task->ctx.ia32eCtx.ksp = cpu->syscallStacks.syscallKsp;
    task->ctx.ia32eCtx.usp = cpu->syscallStacks.syscallUsp;
    task->ctx.ia32eCtx.syscallRegs = cpu->syscallRegs;

    task->ctx.ia32eCtx.userGsbase = __ia32eRdmsr(IA32E_KERNEL_GS_BASE);
    task->ctx.ia32eCtx.userFsbase = __ia32eRdmsr(IA32E_FS_BASE);

    task->ctx.ia32eCtx.r15 = topFrame->regs.r15;
    task->ctx.ia32eCtx.r14 = topFrame->regs.r14;
    task->ctx.ia32eCtx.r13 = topFrame->regs.r13;
    task->ctx.ia32eCtx.r12 = topFrame->regs.r12;
    task->ctx.ia32eCtx.r11 = topFrame->regs.r11;
    task->ctx.ia32eCtx.r10 = topFrame->regs.r10;
    task->ctx.ia32eCtx.r9 = topFrame->regs.r9;
    task->ctx.ia32eCtx.r8 = topFrame->regs.r8;
    task->ctx.ia32eCtx.rsi = topFrame->regs.rsi;
    task->ctx.ia32eCtx.rdi = topFrame->regs.rdi;
    task->ctx.ia32eCtx.rdx = topFrame->regs.rdx;
    task->ctx.ia32eCtx.rcx = topFrame->regs.rcx;
    task->ctx.ia32eCtx.rbx = topFrame->regs.rbx;
    task->ctx.ia32eCtx.rax = topFrame->regs.rax;
    task->ctx.ia32eCtx.rbp = topFrame->regs.rbp;
    task->ctx.ia32eCtx.rsp = topFrame->rsp;
    task->ctx.ia32eCtx.rflags = topFrame->rflags;
    task->ctx.ia32eCtx.rip = topFrame->rip;
    task->ctx.ia32eCtx.cs = topFrame->cs;
    task->ctx.ia32eCtx.ss = topFrame->ss;

#if CONFIG_IA32E_VTX

    K_DYNAMIC_ASSERT(task->taggedInfo.type != K_TASK_INVALID);

    if (task->taggedInfo.type != K_TASK_IDLE && task->domain.curDomain->archInfo.ia32eInfo.vm)
        ia32eCpuVtxTaskSaveCtx(task);

#endif
}

void ia32eCpuTaskRestoreCtx(kSchedTask_t *task)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eFrame_t *topFrame = NULL;

    cpu = ia32eThisCpuData();
    topFrame = cpu->topFrame;

    topFrame->regs.fxsaveRegion = task->ctx.ia32eCtx.fpCtx;

    __ia32eWriteDr0(task->ctx.ia32eCtx.dr0);

    cpu->syscallStacks.syscallKsp = task->ctx.ia32eCtx.ksp;
    cpu->syscallStacks.syscallUsp = task->ctx.ia32eCtx.usp;
    cpu->syscallRegs = task->ctx.ia32eCtx.syscallRegs;

    __ia32eWrmsr(IA32E_KERNEL_GS_BASE, task->ctx.ia32eCtx.userGsbase);
    __ia32eWrmsr(IA32E_FS_BASE, task->ctx.ia32eCtx.userFsbase);

    topFrame->regs.r15 = task->ctx.ia32eCtx.r15;
    topFrame->regs.r14 = task->ctx.ia32eCtx.r14;
    topFrame->regs.r13 = task->ctx.ia32eCtx.r13;
    topFrame->regs.r12 = task->ctx.ia32eCtx.r12;
    topFrame->regs.r11 = task->ctx.ia32eCtx.r11;
    topFrame->regs.r10 = task->ctx.ia32eCtx.r10;
    topFrame->regs.r9  = task->ctx.ia32eCtx.r9;
    topFrame->regs.r8  = task->ctx.ia32eCtx.r8;
    topFrame->regs.rsi = task->ctx.ia32eCtx.rsi;
    topFrame->regs.rdi = task->ctx.ia32eCtx.rdi;
    topFrame->regs.rdx = task->ctx.ia32eCtx.rdx;
    topFrame->regs.rcx = task->ctx.ia32eCtx.rcx;
    topFrame->regs.rbx = task->ctx.ia32eCtx.rbx;
    topFrame->regs.rax = task->ctx.ia32eCtx.rax;
    topFrame->regs.rbp = task->ctx.ia32eCtx.rbp;
    topFrame->rsp = task->ctx.ia32eCtx.rsp;
    topFrame->rflags = task->ctx.ia32eCtx.rflags;
    topFrame->rip = task->ctx.ia32eCtx.rip;
    topFrame->cs = task->ctx.ia32eCtx.cs;
    topFrame->ss = task->ctx.ia32eCtx.ss;

#if CONFIG_IA32E_VTX

    K_DYNAMIC_ASSERT(task->taggedInfo.type != K_TASK_INVALID);

    if (task->taggedInfo.type != K_TASK_IDLE && task->domain.curDomain->archInfo.ia32eInfo.vm)
        ia32eCpuVtxTaskRestoreCtx(task);
    
#endif
}

uintptr_t ia32eCpuSyscallGetReturnAddress(void)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eRegs_t *syscallRegs = NULL;

    cpu = ia32eThisCpuData();
    syscallRegs = cpu->syscallRegs;

    return syscallRegs->rcx;
}

void ia32eCpuSyscallSetReturnAddress(uintptr_t returnAddress)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eRegs_t *syscallRegs = NULL;

    cpu = ia32eThisCpuData();
    syscallRegs = cpu->syscallRegs;

    syscallRegs->rcx = returnAddress;
}


void ia32eCpuExceptionSetReturnAddress(uintptr_t returnAddress)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eFrame_t *frame = NULL;

    cpu = ia32eThisCpuData();
    frame = cpu->currentFrame;

    frame->rip = returnAddress;
}

void ia32eCpuEnterDomain(kDomain_t *domain)
{
    uint64_t cr3 = 0;
    ia32ePerCpu_t *cpu = NULL;

#if CONFIG_IA32E_FEATURE_PCID && CONFIG_KMAX_DOMAINS > 4096
    uint32_t pcid = 0;
#endif

    cr3 = domain->archInfo.ia32eInfo.cr3;
    cpu = ia32eThisCpuData();

#if CONFIG_IA32E_VTX

    if (domain->archInfo.ia32eInfo.vm) {
        ia32eCpuVtxEnterDomain(domain);
        return;
    }

#endif

#if CONFIG_IA32E_FEATURE_PCID 

#   if CONFIG_KMAX_DOMAINS > 4096
    
    cr3 = domain->archInfo.ia32eInfo.cr3;
    cpu = ia32eThisCpuData();

    if (cpu->cpuFlags.fields.pcid != 0) {

        pcid = domain->archInfo.ia32eInfo.cr3 & 0xfff;

        if (cpu->pcidLastDomain[pcid] && cpu->pcidLastDomain[pcid] != domain)
            __ia32eInvpcid(IA32E_INVPCID_CTX, pcid, 0);

        cpu->pcidLastDomain[pcid] = domain;
    }

#   endif 

    if (cpu->cpuFlags.fields.pcid == 0)
        cr3 &= ~0xfff;

#endif

    __ia32eWriteCr3(cr3);
    memcpy(cpu->cpuDataStructures.tssFull.iopb, domain->archInfo.ia32eInfo.iopb, sizeof(domain->archInfo.ia32eInfo.iopb));
}

void ia32eCpuTaskLsrPush(kSchedTask_t *task)
{
    kSchedLsr_t *lsr = NULL;
    ia32ePerCpu_t *thisCpu = NULL;
    uint32_t cpuId = 0;

    ia32ePerCpu_t *targetCpu = NULL;
    uint8_t vector = 0;

    lsr = &task->taggedInfo.info.lsr;
    thisCpu = ia32eThisCpuData();
    cpuId = task->cpuId;

    K_DYNAMIC_ASSERT(kCpuIdValidate(cpuId));

    targetCpu = &thisCpu->global->cpuTable[cpuId];
    vector = lsr->archInfo.ia32eInfo.vector;
    
    stackqPush(&targetCpu->lsrs[vector], &lsr->node);
}

uint32_t ia32eEventSender(void)
{
    return ia32eThisCpuData()->global->ipiData.ipiSender;
}

bool ia32eCpuIdValidate(uint32_t cpuId)
{
    ia32eGlobal_t *global = NULL;

    global = ia32eThisCpuData()->global;

    K_DYNAMIC_ASSERT(global->numCpus <= ARRAY_LEN(global->cpuTable));

    return cpuId < global->numCpus && global->cpuTable[cpuId].cpuFlags.fields.online != 0;
}

int ia32eCpuThreadInfoInit(archSchedThreadInfo_t *info, archSchedThreadParam_t *param)
{
#if CONFIG_IA32E_VTX
    int ret = 0;
#endif

    if (param->ia32eParam.tpr >= IA32E_MAX_VECTOR_PRIO)
        return -EINVAL;

#if CONFIG_IA32E_VTX
    
    if (ia32eCpuVtxThreadParamIsVm(param)) {

        ret = ia32eCpuVtxThreadInfoInit(info, param);
        if (ret < 0)
            return ret;
    } 

#endif

    info->ia32eInfo.tpr = param->ia32eParam.tpr > 1 ? param->ia32eParam.tpr : 1;
    return 0;
}

int ia32eCpuLsrInfoInit(archSchedLsrInfo_t *info, archSchedLsrParam_t *param)
{
    uint8_t vector = 0;
    uint8_t prio = 0;

#if CONFIG_IA32E_VTX
    int ret = 0;
#endif

    vector = param->ia32eParam.vector;
    prio = IA32E_VECTOR_TO_PRIO(vector);

    if (prio <= 1 || prio == IA32E_MAX_VECTOR_PRIO)
        return -EINVAL;


#if CONFIG_IA32E_VTX
    
    if (ia32eCpuVtxLsrParamIsVm(param)) {

        ret = ia32eCpuVtxLsrInfoInit(info, param);
        if (ret < 0)
            return ret;
    } 

#endif

    info->ia32eInfo.vector = vector;
    return 0;
}

int ia32eCpuDomainInfoInit(archDomainInfo_t *info, archDomainParam_t *param)
{
    uintptr_t pml4Phys = 0;
    ia32ePml4_t *pml4Virt = NULL;
    ia32ePml4e_t pml4e = 0;

#if CONFIG_IA32E_FEATURE_PCID
    ia32ePerCpu_t *cpu = ia32eThisCpuData();
    ia32eGlobal_t *global = NULL; 
    int32_t pcid = 0;

    global = cpu->global;
#endif

#if CONFIG_IA32E_VTX

    if (param->ia32eParam.vm)
        return ia32eCpuVtxDomainInfoInit(info, param);

#endif

    pml4Phys = param->ia32eParam.pml4BasePhys;
    pml4Virt = param->ia32eParam.pml4BaseVirt;

    if ((pml4Phys & 0xfff) != 0 || ((uintptr_t)pml4Virt & 0xfff) != 0)
        return -EINVAL;

    if ((pml4Virt->pml4e[511] & IA32E_PG_ENTRY_PRESENT_MASK) != 0)
        return -EINVAL;

    pml4e |= (uintptr_t)ia32ePdpt;
    pml4e |= IA32E_PG_ENTRY_PRESENT_MASK;
    pml4e |= IA32E_PG_ENTRY_RW_MASK;
    pml4e |= IA32E_PG_ENTRY_GLOBAL_MASK;
    pml4Virt->pml4e[511] = pml4e;

    STATIC_ASSERT(sizeof(info->ia32eInfo.iopb) == sizeof(param->ia32eParam.iopb));

    memcpy(info->ia32eInfo.iopb, param->ia32eParam.iopb, sizeof(info->ia32eInfo.iopb));

    info->ia32eInfo.cr3 = pml4Phys;

#if CONFIG_IA32E_FEATURE_PCID

    if (global->gFlags.fields.pcidCapableExists != 0) {

        pcid = global->pcidCtr % 4096;
        global->pcidCtr++;

        info->ia32eInfo.cr3 |= pcid;
    }

#endif

    return 0;
}

/* Timer */

uint32_t ia32eTimerFrequencyHz(void)
{
    ia32eGlobal_t *global = NULL;
    uint32_t sender = 0;

    global = ia32eThisCpuData()->global;
    sender = global->ipiData.ipiSender;

    return global->cpuTable[sender].apicFrequencyHz;
}

void ia32eTimerArmPeriodic(uint32_t ticks)
{
    uint64_t timer = 0;

    timer = IA32E_K_EVENT_VECTOR | (IA32E_XAPIC_PERIODIC << 17);

    ia32eApicWrite(IA32E_XAPIC_DCR_OFFSET, IA32E_XAPIC_DIV_16, false);
    ia32eApicWrite(IA32E_XAPIC_TIMER_OFFSET, timer, false);
    ia32eApicWrite(IA32E_XAPIC_INITIAL_COUNT_OFFSET, ticks, false);
}

/* Callback */

void ia32eCallbackActivation(kSchedTask_t *task)
{
    ia32eGlobal_t *global = NULL;
    ia32ePerCpu_t *cpu = NULL;

    kSchedThread_t *thread = NULL;
    kSchedLsr_t *lsr = NULL;

    bool found = false;
    uint8_t tpr = 0;

    K_DYNAMIC_ASSERT(kCpuIdValidate(task->cpuId));

    global = ia32eGetGlobalPtr();
    cpu = &global->cpuTable[task->cpuId];

    switch (task->taggedInfo.type) {
        
        case K_TASK_THREAD:
            thread = &task->taggedInfo.info.thread;
            found = true;
            tpr = thread->archInfo.ia32eInfo.tpr;
            K_DYNAMIC_ASSERT(tpr > 0 && tpr < IA32E_MAX_VECTOR_PRIO);
            break;
        
        case K_TASK_LSR:
            lsr = &task->taggedInfo.info.lsr;
            found = true;
            tpr = IA32E_VECTOR_TO_PRIO(lsr->archInfo.ia32eInfo.vector);
            K_DYNAMIC_ASSERT(tpr > 1 && tpr < IA32E_MAX_VECTOR_PRIO);
            break;
        
        default:
            break;
    }

    if (found) {
        K_DYNAMIC_ASSERT(cpu->tprCount[tpr - 1] < UINT32_MAX);
        cpu->tprCount[tpr - 1]++; 

        if (gPluginsDone && tpr > cpu->tpr)
            ia32eReloadTpr();
    }
}

void ia32eCallbackResponse(kSchedTask_t *task)
{
    ia32ePerCpu_t *cpu = NULL;
    kSchedThread_t *thread = NULL;
    kSchedLsr_t *lsr = NULL;

    bool found = false;
    uint8_t tpr = 0;

    cpu = ia32eThisCpuData();
    
    switch (task->taggedInfo.type) {
        
        case K_TASK_THREAD:
            thread = &task->taggedInfo.info.thread;
            found = true;
            tpr = thread->archInfo.ia32eInfo.tpr;
            K_DYNAMIC_ASSERT(tpr > 0 && tpr < IA32E_MAX_VECTOR_PRIO);
            break;
        
        case K_TASK_LSR:
            lsr = &task->taggedInfo.info.lsr;
            found = true;
            tpr = IA32E_VECTOR_TO_PRIO(lsr->archInfo.ia32eInfo.vector);
            K_DYNAMIC_ASSERT(tpr > 1 && tpr < IA32E_MAX_VECTOR_PRIO);
            break;
        
        default:
            break;
    }

    if (found) {

        K_DYNAMIC_ASSERT(tpr <= cpu->tpr);
        K_DYNAMIC_ASSERT(cpu->tprCount[tpr - 1] > 0);
        
        cpu->tprCount[tpr - 1]--; 

        if (tpr == cpu->tpr && cpu->tprCount[tpr - 1] == 0)
            ia32eReloadTpr();
    }
}

void ia32eCallbackCpuHandoff(void)
{
    ia32eReloadTpr();   
}

/* Event */

void ia32eIsrPreHandler(ia32eFrame_t *frame)
{
    if (IA32E_SELECTOR_TO_RPL(frame->cs) != 0) {
        __ia32eSwapgs();
        barrier();
    }
}

void ia32eEarlyPanic(const char *str)
{
    kDbgStr(str);
    __ia32eHltForever();

    UNREACHABLE();
}

void ia32eFlushTlbAll(void)
{
    ia32eCpuInvokeAllRendezvous(ia32eFlushTlb);
}

void __ia32eLogExceptionKernel(ia32eFrame_t *frame)
{
    uint8_t vector = 0;
    char *exceptionStr = NULL;

    vector = frame->vector;
    exceptionStr = vector < ARRAY_LEN(vectorToStr) ? vectorToStr[vector] : "UNKNOWN";

    kDbgStr("TRACE:\n");

    kDbgStrf("[EXCEPTION]: %s\n", exceptionStr);

    kDbgStrf("[ERRCODE]: 0x%lx\n", frame->errcode);
    kDbgStrf("[RIP]: 0x%lx\n", frame->rip);
    kDbgStrf("[RSP]: 0x%lx\n", frame->rsp);

    kDbgStrf("[RBP]: 0x%lx\n", frame->regs.rbp);
    kDbgStrf("[RAX]: 0x%lx\n", frame->regs.rax);
    kDbgStrf("[RBX]: 0x%lx\n", frame->regs.rbx);
    kDbgStrf("[RCX]: 0x%lx\n", frame->regs.rcx);
    kDbgStrf("[RDX]: 0x%lx\n", frame->regs.rdx);
    kDbgStrf("[RDI]: 0x%lx\n", frame->regs.rdi);
    kDbgStrf("[RSI]: 0x%lx\n", frame->regs.rsi);
    kDbgStrf("[R8]: 0x%lx\n", frame->regs.r8);
    kDbgStrf("[R9]: 0x%lx\n", frame->regs.r9);
    kDbgStrf("[R10]: 0x%lx\n", frame->regs.r10);
    kDbgStrf("[R11]: 0x%lx\n", frame->regs.r11);
    kDbgStrf("[R12]: 0x%lx\n", frame->regs.r12);
    kDbgStrf("[R13]: 0x%lx\n", frame->regs.r13);
    kDbgStrf("[R14]: 0x%lx\n", frame->regs.r14);
    kDbgStrf("[R15]: 0x%lx\n", frame->regs.r15);

    kDbgStrf("[RFLAGS]: 0x%lx\n", frame->rflags);
    kDbgStrf("[CS]: 0x%lx\n", frame->cs);
    kDbgStrf("[SS]: 0x%lx\n", frame->ss);

    kDbgStrf("[CR0]: 0x%lx\n", __ia32eReadCr0());
    kDbgStrf("[CR2]: 0x%lx\n", __ia32eReadCr2());
    kDbgStrf("[CR3]: 0x%lx\n", __ia32eReadCr3());
    kDbgStrf("[CR4]: 0x%lx\n", __ia32eReadCr4());
    kDbgStrf("[CR8]: 0x%lx\n", __ia32eReadCr8());

    kDbgStrf("[FSBASE]: 0x%lx\n", __ia32eRdmsr(IA32E_FS_BASE));
    kDbgStrf("[GSBASE]: 0x%lx\n", __ia32eRdmsr(IA32E_GS_BASE));

    kDbgStrf("[DR6]: 0x%lx\n", __ia32eReadDr6());
    kDbgStrf("[DR7]: 0x%lx\n", __ia32eReadDr7());
}

void ia32eEarlyExceptionHandler(ia32eFrame_t *frame)
{
    mcsNode_t node = {0};

    __mcsNodeInit(&node);
    __mcsAcquire(&exceptionLock, &node);
    __ia32eLogExceptionKernel(frame);
    __mcsRelease(&exceptionLock, &node);

    __ia32eHltForever();
    UNREACHABLE();
}

void ia32eExceptionHandlerUser(void)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eFrame_t *frame = NULL;
    uint8_t vector = 0;
    kDomainInvocationType_t type = K_INVOCATION_INVALID;
    uintptr_t vmemFaultAddress = 0;

    uint64_t dr6 = 0;

    cpu = ia32eThisCpuData();
    frame = cpu->currentFrame;
    vector = frame->vector;

    switch (vector) {

        case IA32E_PAGE_FAULT:
            type = K_INVOCATION_EXCEPTION_VMEM_FAULT;
            vmemFaultAddress = __ia32eReadCr2();
            break;

        case IA32E_INVALID_OPCODE:
            type = K_INVOCATION_EXCEPTION_ILLEGAL_OPCODE;
            break;

        case IA32E_ALIGNMENT_CHECK:
            type = K_INVOCATION_EXCEPTION_ALIGNMENT;
            break;

        case IA32E_DEBUG_EXCEPTION:
            dr6 = __ia32eReadDr6();
            dr6 &= ~IA32E_DR6_STICKY_MASK;
            __ia32eWriteDr6(dr6);
            ATTR_FALLTHROUGH;

        case IA32E_BREAKPOINT:
            type = K_INVOCATION_EXCEPTION_DEBUG;
            break;

        case IA32E_DIVIDE_ERROR:
        case IA32E_MATH_FAULT:
        case IA32E_SIMD_FLOATING_POINT_EXCEPTION:
            type = K_INVOCATION_EXCEPTION_ARITHMETIC;
            break;

        default:
            type = K_INVOCATION_EXCEPTION_OTHER;
            break;
    }

    kExceptionHandler(false, type, frame->rip, vmemFaultAddress, frame->errcode);
}

void ia32eExceptionHandlerKernel(void)
{
    ia32ePerCpu_t *cpu = NULL;
    uint32_t cpuId = 0;
    ia32eFrame_t *frame = NULL;
    uint64_t dr6 = 0;
    bool stackOverflow = false;
    bool wakeupStackOverlow = false;
    bool syscallStackOverflow = false;
    bool intStackOverflow = false;
    bool nmiStackOverflow = false;
    bool doubleFaultStackOverflow = false;
    uintptr_t wakeupPadding = 0;

    mcsNode_t node = {0};

    cpu = ia32eThisCpuData();
    cpuId = cpu->cpuId;
    frame = cpu->currentFrame;

    if (frame->vector == IA32E_DEBUG_EXCEPTION) {

        dr6 = __ia32eReadDr6();

        if ((dr6 & IA32E_KERNEL_STACK_OF_DR6_MASK) != 0) {
            
            stackOverflow = true;

            wakeupPadding = cpu->cpuFlags.fields.bsp != 0 ? (uintptr_t)ia32eBootStackPadding : 
                                                            (uintptr_t)cpu->wakeupStack.padding;

            if (__ia32eReadDr0() == wakeupPadding)
                wakeupStackOverlow = true;
            else 
                syscallStackOverflow = true;
        }
        
        if ((dr6 & IA32E_INT_STACK_OF_DR6_MASK) != 0) {
            stackOverflow = true;
            intStackOverflow = true;
        }

        if ((dr6 & IA32E_NMI_STACK_OF_DR6_MASK) != 0) {
            stackOverflow = true;
            nmiStackOverflow = true;
        }

        if ((dr6 & IA32E_DOUBLE_FAULT_STACK_OF_DR6_MASK) != 0) {
            stackOverflow = true;
            doubleFaultStackOverflow = true;
        }

        dr6 &= ~IA32E_DR6_STICKY_MASK;
        __ia32eWriteDr6(dr6);
    }

    __mcsNodeInit(&node);
    __mcsAcquire(&exceptionLock, &node);

    kDbgStrf("[CPU]: %u\n", cpuId);

    if (stackOverflow) {
        kDbgStr("[STACK OVERFLOW DETECTED]\n");

        if (wakeupStackOverlow)
            kDbgStr("[STACK OVERFLOW ON WAKEUP STACK]\n");

        if (syscallStackOverflow)
            kDbgStr("[STACK OVERFLOW ON SYSCALL STACK]\n");

        if (intStackOverflow)
            kDbgStr("[STACK OVERFLOW ON INTERRUPT STACK]\n");

        if (nmiStackOverflow)
            kDbgStr("[STACK OVERFLOW ON NMI STACK]\n");

        if (doubleFaultStackOverflow)
            kDbgStr("[STACK OVERFLOW ON DOUBLE FAULT STACK]\n");
    }

    __ia32eLogExceptionKernel(frame);
    __mcsRelease(&exceptionLock, &node);

    __ia32eHltForever();
    UNREACHABLE();
}

void ia32eIsrExternalHandler(void)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eFrame_t *frame = NULL;
    stackq_t *stackq = NULL;
    bool pushed = false;

    stackqNode_t *node = NULL; 
    kSchedTask_t *task = NULL;

    cpu = ia32eThisCpuData();
    frame = cpu->currentFrame;
    stackq = &cpu->lsrs[frame->vector];

    while ((node = stackqPop(stackq)) != NULL) {

        if (!pushed) {
            kLsrHandler(K_LSR_OP_PUSH_CURRENT, NULL);
            pushed = true;
        }

        task = kSchedTaskFromLsrStackqNode(node);

        K_DYNAMIC_ASSERT(task->taggedInfo.type == K_TASK_LSR);
        K_DYNAMIC_ASSERT(task->state == K_TASK_STATE_LSR_DORMANT);

        kLsrHandler(K_LSR_OP_PUSH, &task->taggedInfo.info.lsr);
    }
    
    if (pushed)
        kLsrHandler(K_LSR_OP_RESCHEDULE, NULL);
}

void ia32eIsrExceptionHandler(void)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eGlobal_t *global = NULL;
    ia32eFrame_t *frame = NULL;
    kCpuInvokeRoutineFn_t nmiFn = NULL;

    cpu = ia32eThisCpuData();
    global = cpu->global;
    frame = cpu->currentFrame;
    nmiFn = global->nmiHandler;

    if (frame->vector == IA32E_NMI) {

        if (nmiFn)
            nmiFn();

        return;
    }

    if (IA32E_SELECTOR_TO_RPL(frame->cs) == 0) {
        ia32eExceptionHandlerKernel();
        UNREACHABLE();
    }
    
    ia32eExceptionHandlerUser();
}

void ia32eIsrEventHandler(void)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eGlobal_t *global = NULL;
    kCpuInvokeRoutineFn_t fn = NULL;

    cpu = ia32eThisCpuData();
    global = cpu->global;

    if (cpu->cpuId != global->ipiData.ipiSender) {

        fn = global->ipiData.fn;

        if (fn) {
            fn();
            atomic_fetch_add(&global->ipiData.rendezvous, 1);
        }

        return;
    }

    kTickHandler();
}

void ia32eIsrMain(void)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eFrame_t *frame = NULL;
    bool external = false;
    uint8_t vector = 0;
    kCpuInvokeRoutineFn_t selfIpiFn = NULL;

    cpu = ia32eThisCpuData();
    frame = cpu->currentFrame;
    external = cpu->external;
    vector = frame->vector;
    selfIpiFn = cpu->selfIpiFn;

    switch (vector) {

        case IA32E_SPURIOUS_INT_VECTOR:
            break;

        case IA32E_K_EVENT_VECTOR:
            ia32eIsrEventHandler();
            break;

        case IA32E_K_FAKE_ISR_VECTOR:
            
            if (selfIpiFn) {
                selfIpiFn();
                cpu->selfIpiFn = NULL;
            }

            break;

        default:

            if (external) 
                ia32eIsrExternalHandler();
            else 
                ia32eIsrExceptionHandler();

            break;
    }
}

bool ia32eIsrHandler(ia32eFrame_t *frame)
{
    uint8_t vector = 0;
    ia32ePerCpu_t *cpu = NULL;
    bool wasInIsr = false;
    bool wasExternal = false;
    bool external = false;
    ia32eFrame_t *oldFrame = NULL;

    if (!ia32eGlobal.intcSetup) {

        if (vector >= ARRAY_LEN(interruptTypes) || interruptTypes[vector] != IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION)
            return IA32E_SELECTOR_TO_RPL(frame->cs) != 0;

        ia32eEarlyExceptionHandler(frame);
        UNREACHABLE();
    }

    vector = frame->vector;
    cpu = ia32eThisCpuData();
    wasInIsr = cpu->inIsr;
    wasExternal = cpu->external;
    external = !wasInIsr && ia32eApicCheckIsr(vector);
    oldFrame = cpu->currentFrame;

    cpu->inIsr = true;
    cpu->external = external;
    cpu->currentFrame = frame;
    if (!wasInIsr)
        cpu->topFrame = frame;
    
    ia32eIsrMain();

    if (external)
        ia32eApicEoi();

    cpu->inIsr = wasInIsr;
    cpu->external = wasExternal;
    cpu->currentFrame = oldFrame;
    if (!wasInIsr)
        cpu->topFrame = NULL;

    barrier();

    return IA32E_SELECTOR_TO_RPL(frame->cs) != 0;
}

void ia32eSyscallHandler(ia32eRegs_t *regs)
{
    ia32ePerCpu_t *cpu = NULL;
    uintptr_t id = 0;
    uintptr_t param1 = 0; 
    uintptr_t param2 = 0;
    intptr_t ret = 0;

    cpu = ia32eThisCpuData();
   
    id = regs->rax;
    param1 = regs->rdi;
    param2 = regs->rsi;
  
    cpu->syscallRegs = regs;
    ret = kSyscallHandler(id, param1, param2);
    cpu->syscallRegs = NULL;

    regs->rax = ret;
}