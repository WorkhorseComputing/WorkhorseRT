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

#include <ia32eCpuVtx.h>

#if CONFIG_IA32E_VTX

#include <ia32eCpu.h>
#include <ia32eHpet.h>
#include <ia32eVma.h>
#include <lib/acpi.h>
#include <export/kDbgInterface.h>
#include <plugin/kPlugin.h>
#include <workhorse/kDomainUniverse/kDomainUniverse.h>

extern 
void __ia32eVmlaunchStub(void);

extern 
void __ia32eVmexitStub(void);

static
uint32_t ia32eVtxVmcsMsrAllowList[] = {
    IA32E_SYSENTER_CS,
    IA32E_SYSENTER_ESP,
    IA32E_SYSENTER_EIP,

    IA32E_EFER,
    IA32E_PAT,

    IA32E_GS_BASE,
    IA32E_FS_BASE,

    IA32E_STAR,
    IA32E_LSTAR,
    IA32E_CSTAR,
    IA32E_FMASK,

    IA32E_KERNEL_GS_BASE
};

static
uint32_t ia32eVtxVmcsMsrCtxList[] = {
    IA32E_STAR,
    IA32E_LSTAR,
    IA32E_CSTAR,
    IA32E_FMASK,

    IA32E_KERNEL_GS_BASE    
};

static
inline 
void ia32eVtxVmcsMapMsrWrite(uint32_t msr, int32_t *base, int32_t *idx)
{
    if (msr <= 0x1fff) {

        *base = IA32E_VTX_VMCS_MSR_WRITE_LOW;
        *idx = msr;

    } else if (msr >= 0xc0000000 && msr <= 0xc0001fff) {

        *base = IA32E_VTX_VMCS_MSR_WRITE_HIGH;
        *idx = msr - 0xc0000000;

    } else {

        *base = -1;
        *idx = -1;
    }
}

static
inline 
void ia32eVtxVmcsMapMsrRead(uint32_t msr, int32_t *base, int32_t *idx)
{
    if (msr <= 0x1fff) {

        *base = IA32E_VTX_VMCS_MSR_READ_LOW;
        *idx = msr;

    } else if (msr >= 0xc0000000 && msr <= 0xc0001fff) {

        *base = IA32E_VTX_VMCS_MSR_READ_HIGH;
        *idx = msr - 0xc0000000;

    } else {

        *base = -1;
        *idx = -1;
    }
}

static
inline 
void ia32eVtxVmcsUntrapMsrWrite(char *bitmap, uint32_t msr)
{
    int32_t base = 0;
    int32_t idx = 0;

    int32_t bitmapIdx = 0;

    ia32eVtxVmcsMapMsrWrite(msr, &base, &idx);

    K_DYNAMIC_ASSERT(base >= 0 && idx >= 0);

    if (base >= 0 && idx >= 0) {

        bitmapIdx = base + (idx/8);

        K_DYNAMIC_ASSERT(bitmapIdx >= 0 && bitmapIdx < 4096);

        bitmap[bitmapIdx] &= ~(1 << (idx % 8));   
    }
}

static 
inline 
void ia32eVtxVmcsUntrapMsrRead(char *bitmap, uint32_t msr)
{
    int32_t base = 0;
    int32_t idx = 0;

    int32_t bitmapIdx = 0;

    ia32eVtxVmcsMapMsrRead(msr, &base, &idx);

    K_DYNAMIC_ASSERT(base >= 0 && idx >= 0);

    if (base >= 0 && idx >= 0) {

        bitmapIdx = base + (idx/8);

        K_DYNAMIC_ASSERT(bitmapIdx >= 0 && bitmapIdx < 4096);
   
        bitmap[bitmapIdx] &= ~(1 << (idx % 8));   
    }
}

static
inline 
void ia32eVtxVmcsUntrapMsr(char *bitmap, uint32_t msr)
{
    ia32eVtxVmcsUntrapMsrRead(bitmap, msr);
    ia32eVtxVmcsUntrapMsrWrite(bitmap, msr);
}

static
inline 
bool ia32eVmwriteAdjusted(uint32_t msr, uint64_t field, uint64_t val)
{
    uint64_t cap = 0;
    
    cap = __ia32eRdmsr(msr);
    
    val |= (cap & 0xffffffff);
    val &= (cap >> 32);

    return __ia32eVmwrite(field, val);
}

static
inline
bool ia32eCpuVtxIsVcpuCapable(uint32_t cpuId)
{
    ia32eGlobal_t *global = NULL;

    global = ia32eThisCpuData()->global;

    K_DYNAMIC_ASSERT(kCpuIdValidate(cpuId));

    return global->cpuTable[cpuId].cpuFlags.fields.vcpuCapable != 0;
}

static
inline
int ia32eCpuVtxTaskInfoInit(ia32eVtxTaskInfo_t *info, uint32_t domId , ia32eVtxParam_t *param)
{
    kDomain_t *domain = NULL;

    if ((param->vmcsPhys & 0xfff) != 0 || ((uintptr_t)param->vmcsVirt & 0xfff) != 0)
        return -EINVAL;

    domain = kDomainUniverseGet(domId);

    K_DYNAMIC_ASSERT(domain->archInfo.ia32eInfo.numVcpus < UINT32_MAX);

    info->vtxParam = *param;
    info->vcpuId = domain->archInfo.ia32eInfo.numVcpus;

    dqPushBack(&domain->archInfo.ia32eInfo.vcpuVector, &info->vcpuVectorNode);
    domain->archInfo.ia32eInfo.numVcpus++;
    
    return 0;
}

static
uint32_t ia32eVtxCalibrateTscManual(void)
{
    ia32eGlobal_t *global = NULL;    

    volatile uint32_t *acpiPmMmio = NULL;
    uint16_t acpiPmPort = 0;

    uint32_t counterFrequencyHz = 0;
    uint64_t calibrationTicks = 0;
    uint64_t startTicks = 0;

    uint32_t tscCur = 0;
    uint32_t tscTicksElapsed = 0;
    uint32_t tscFrequencyHz = 0;

    global = ia32eThisCpuData()->global;

    if (ia32eHpetIsInitialized()) {

        counterFrequencyHz = ia32eHpetFrequencyHz();
        calibrationTicks = msToTicks(CONFIG_IA32E_VTX_TSC_CALIBRATION_TIME_MS, counterFrequencyHz);

        tscCur = __ia32eRdtsc();
        startTicks = ia32eHpetReadCounter();
        spinUntil(ia32eHpetReadCounter() - startTicks >= calibrationTicks);

    } else {

        counterFrequencyHz = ACPI_PM_TMR_HZ;
        calibrationTicks = msToTicks(CONFIG_IA32E_VTX_TSC_CALIBRATION_TIME_MS, counterFrequencyHz);

        if (global->acpiPm.mmio) {

            acpiPmMmio = (void *)global->acpiPm.acpiPmMmio;

            tscCur = __ia32eRdtsc();
            startTicks = READ_ONCE(*acpiPmMmio);
            spinUntil(READ_ONCE(*acpiPmMmio) - startTicks >= calibrationTicks);

        } else {
            acpiPmPort = (uint16_t)global->acpiPm.acpiPmPort;

            tscCur = __ia32eRdtsc();
            startTicks = __ia32eInl(acpiPmPort);
            spinUntil( __ia32eInl(acpiPmPort) - startTicks >= calibrationTicks);
        }
    }

    tscTicksElapsed = __ia32eRdtsc() - tscCur;
    tscFrequencyHz = (tscTicksElapsed * counterFrequencyHz) / calibrationTicks;   
    
    if (tscFrequencyHz == 0) {
        ia32eEarlyKpanic("failed to calibrate tsc, ended up with a frequency of 0 after manual calibration\n");
        UNREACHABLE();
    }

    return tscFrequencyHz;
}

static
void ia32eCpuVtxVmcsSetup(kSchedTask_t *task)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eGlobal_t *global = NULL;

    kSchedThread_t *thread = NULL;
    kSchedLsr_t *lsr = NULL;

    ia32eVtxVmcsRegion_t *vmcsVirt = NULL;
    uintptr_t vmcsPhys = 0;

    uint32_t exceptionBitmap = 0;
    uint32_t pin = 0;
    uint32_t proc = 0;
    uint32_t exit = 0;
    uint32_t entry = 0;
    uint32_t proc2 = 0;

    uint64_t cr0Mask = 0;
    uint64_t cr0Shadow = 0;
    
    uint64_t cr4Mask = 0;
    
    cpu = ia32eThisCpuData();
    global = cpu->global;

    switch (task->taggedInfo.type) {
        
        case K_TASK_THREAD:
            thread = &task->taggedInfo.info.thread;
            vmcsVirt = thread->archInfo.ia32eInfo.vtxInfo.vtxParam.vmcsVirt;
            vmcsPhys = thread->archInfo.ia32eInfo.vtxInfo.vtxParam.vmcsPhys;
            break;

        case K_TASK_LSR:
            lsr = &task->taggedInfo.info.lsr;
            vmcsVirt = lsr->archInfo.ia32eInfo.vtxInfo.vtxParam.vmcsVirt;
            vmcsPhys = lsr->archInfo.ia32eInfo.vtxInfo.vtxParam.vmcsPhys;
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }

    /* vmx mode */

    vmcsVirt->header = cpu->vtx.revisionId;

#if CONFIG_KDYNAMIC_ASSERT

    K_DYNAMIC_ASSERT(__ia32eVmclear(vmcsPhys));
    K_DYNAMIC_ASSERT(__ia32eVmptrld(vmcsPhys));

#else 

    __ia32eVmclear(vmcsPhys);
    __ia32eVmptrld(vmcsPhys);

#endif

    /* controls */

    exceptionBitmap = IA32E_VTX_VMCS_EXCEPTION_BITMAP_DEBUG_MASK | 
                      IA32E_VTX_VMCS_EXCEPTION_BITMAP_ALIGNMENT_CHECK_MASK;

    pin = (1U << IA32E_VTX_VMCS_PINBASED_CTLS_EXTERNAL_INTERRUPT_EXITING_BIT) |
                (1U << IA32E_VTX_VMCS_PINBASED_CTLS_NMI_EXITING_BIT) |
                (1U << IA32E_VTX_VMCS_PINBASED_CTLS_VIRTUAL_NMIS_BIT);
            
    proc = (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_MWAIT_EXITING_BIT) |
            (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_RDPMC_EXITING_BIT) |

#if CONFIG_IA32E_VTX_TSD
            (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_RDTSC_EXITING_BIT) |
#endif

            (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_IO_BITMAPS_BIT) |
            (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_MSR_BITMAPS_BIT) |
            (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_MONITOR_EXITING_BIT) |
            (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_SECONDARY_CTLS_BIT);
                
    exit = (1U << IA32E_VTX_VMCS_EXIT_CTLS_HOST_ADDRESS_SPACE_SIZE_BIT) |
            (1U << IA32E_VTX_VMCS_EXIT_CTLS_ACKNOWLEDGE_INTERRUPT_ON_EXIT_BIT) |
            (1U << IA32E_VTX_VMCS_EXIT_CTLS_SAVE_PAT_BIT) |
            (1U << IA32E_VTX_VMCS_EXIT_CTLS_LOAD_PAT_BIT) |
            (1U << IA32E_VTX_VMCS_EXIT_CTLS_SAVE_EFER_BIT) |
            (1U << IA32E_VTX_VMCS_EXIT_CTLS_LOAD_EFER_BIT);

    entry = (1U << IA32E_VTX_VMCS_ENTRY_CTLS_LOAD_PAT_BIT) | 
            (1U << IA32E_VTX_VMCS_ENTRY_CTLS_LOAD_EFER_BIT);

    proc2 = (1U << IA32E_VTX_VMCS_PROCBASED_CTLS2_EPT_BIT) |
            (1U << IA32E_VTX_VMCS_PROCBASED_CTLS2_VPID_BIT) |
            (1U << IA32E_VTX_VMCS_PROCBASED_CTLS2_WBINVD_EXITING_BIT) |
            (1U << IA32E_VTX_VMCS_PROCBASED_CTLS2_URG_BIT) |
            (1U << IA32E_VTX_VMCS_PROCBASED_CTLS2_ENCLS_EXITING_BIT);

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_EXCEPTION_BITMAP, exceptionBitmap);
    
    ia32eVmwriteAdjusted(cpu->vtx.ia32eVmxPinbasedCtls, IA32E_VTX_VMCS_CTRL_PINBASED_CONTROLS, pin);
    ia32eVmwriteAdjusted(cpu->vtx.ia32eVmxProcbasedCtls, IA32E_VTX_VMCS_CTRL_PROCBASED_CTLS, proc);
    ia32eVmwriteAdjusted(cpu->vtx.ia32eVmxExitCtls, IA32E_VTX_VMCS_CTRL_PRIMARY_VMEXIT_CONTROLS, exit);
    ia32eVmwriteAdjusted(cpu->vtx.ia32eVmxEntryCtls, IA32E_VTX_VMCS_CTRL_VMENTRY_CONTROLS, entry);

    ia32eVmwriteAdjusted(IA32E_VMX_PROCBASED_CTLS2, IA32E_VTX_VMCS_CTRL_PROCBASED_CTLS2, proc2);

    /* host state */

    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_CR0, __ia32eReadCr0());
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_CR3, __ia32eReadCr3());
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_CR4, __ia32eReadCr4());

    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_CS_SELECTOR, IA32E_KCS_SELECTOR);
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_DS_SELECTOR, IA32E_KDS_SELECTOR);
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_SS_SELECTOR, IA32E_KDS_SELECTOR);
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_ES_SELECTOR, IA32E_KDS_SELECTOR);
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_FS_SELECTOR, IA32E_KDS_SELECTOR);
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_GS_SELECTOR, IA32E_KDS_SELECTOR);
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_TR_SELECTOR, IA32E_TR_SELECTOR);

    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_GS_BASE, __ia32eRdmsr(IA32E_GS_BASE));
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_TR_BASE, (uintptr_t)&cpu->cpuDataStructures.tssFull);
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_GDTR_BASE, cpu->cpuDataStructures.gdtr.base);
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_IDTR_BASE, cpu->global->cpuDataStructures.idtr.base);

    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_RIP, (uintptr_t)__ia32eVmexitStub);
    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_RSP, task->ctx.ia32eCtx.ksp);

    __ia32eVmwrite(IA32E_VTX_VMCS_HOST_IA32E_EFER, __ia32eRdmsr(IA32E_EFER));

    if (cpu->cpuFlags.fields.pat != 0)
        __ia32eVmwrite(IA32E_VTX_VMCS_HOST_IA32E_PAT, __ia32eRdmsr(IA32E_PAT));

    /* guest state */

    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_CR0, IA32E_CR0_NE_MASK);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_CR4, IA32E_CR4_VMXE_MASK);

    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_CS_LIMIT, 0xffff);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_DS_LIMIT, 0xffff);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_SS_LIMIT, 0xffff);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_ES_LIMIT, 0xffff);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_FS_LIMIT, 0xffff);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_GS_LIMIT, 0xffff);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_TR_LIMIT, 0xffff);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_LDTR_LIMIT, 0xffff);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_GDTR_LIMIT, 0xffff);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_IDTR_LIMIT, 0xffff);

    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_CS_ACCESS_RIGHTS, 0x809b);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_DS_ACCESS_RIGHTS, 0x8093);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_SS_ACCESS_RIGHTS, 0x8093);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_ES_ACCESS_RIGHTS, 0x8093);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_FS_ACCESS_RIGHTS, 0x8093);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_GS_ACCESS_RIGHTS, 0x8093);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_TR_ACCESS_RIGHTS, 0x8083);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_LDTR_ACCESS_RIGHTS, 0x10000);

    K_DYNAMIC_ASSERT(task->domain.curDomain->invocationInfo._start <= UINT16_MAX);

    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_RIP, task->domain.curDomain->invocationInfo._start);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_RFLAGS, 2);

    /* shadows */

    cr0Mask = IA32E_CR0_NE_MASK | IA32E_CR0_NW_MASK | IA32E_CR0_CD_MASK;
    cr0Shadow = IA32E_CR0_NE_MASK;

    cr4Mask = IA32E_CR4_TSD_MASK | IA32E_CR4_DE_MASK | IA32E_CR4_PSE_MASK | IA32E_CR4_PAE_MASK | IA32E_CR4_PGE_MASK |
              IA32E_CR4_OSFXSR_MASK | IA32E_CR4_OSXMMEXCPT_MASK;

    if (cpu->cpuFlags.fields.vme != 0)
        cr4Mask |= IA32E_CR4_VME_MASK | IA32E_CR4_PVI_MASK;

    if (cpu->cpuFlags.fields.umip != 0)
        cr4Mask |= IA32E_CR4_UMIP_MASK;

    if (cpu->cpuFlags.fields.fsgsbase != 0)
        cr4Mask |= IA32E_CR4_FSGSBASE_MASK;

    if (cpu->cpuFlags.fields.pcid != 0)
        cr4Mask |= IA32E_CR4_PCIDE_MASK;

    if (cpu->cpuFlags.fields.smep != 0)
        cr4Mask |= IA32E_CR4_SMEP_MASK;

    if (cpu->cpuFlags.fields.smap != 0)
        cr4Mask |= IA32E_CR4_SMAP_MASK;

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_CR0_GUEST_HOST_MASK, cr0Mask);
    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_CR0_READ_SHADOW, cr0Shadow);

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_CR4_GUEST_HOST_MASK, ~cr4Mask);

    /* bitmaps */

    K_DYNAMIC_ASSERT((ia32eVirtToPhysStatic(cpu->vtx.areas.ioBitmap) & 0xfff) == 0);
    K_DYNAMIC_ASSERT((ia32eVirtToPhysStatic(global->vtxGlobal.msrBitmap) & 0xfff) == 0);

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_IO_BITMAP_A, ia32eVirtToPhysStatic(cpu->vtx.areas.ioBitmap));
    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_IO_BITMAP_B, ia32eVirtToPhysStatic(&cpu->vtx.areas.ioBitmap[4096]));
    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_MSR_BITMAPS, ia32eVirtToPhysStatic(global->vtxGlobal.msrBitmap));

    /* msr ctx */

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMEXIT_MSR_LOAD_COUNT, cpu->vtx.areas.msrAreaCount);
    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMEXIT_MSR_STORE_COUNT, cpu->vtx.areas.msrAreaCount);
    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMENTRY_MSR_LOAD_COUNT, cpu->vtx.areas.msrAreaCount);

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMEXIT_MSR_LOAD_ADDRESS, 
                   ia32eVirtToPhysStatic(cpu->vtx.areas.vmexitLoadArea));

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMEXIT_MSR_STORE_ADDRESS, 
                   ia32eVirtToPhysStatic(cpu->vtx.areas.vmexitStoreVmentryLoadArea));

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMENTRY_MSR_LOAD_ADDRESS, 
                   ia32eVirtToPhysStatic(cpu->vtx.areas.vmexitStoreVmentryLoadArea));

    /* sgx */

    if (cpu->cpuFlags.fields.sgx != 0)
        __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_ENCLS_EXITING_BITMAP, UINT64_MAX);

    /* epts */

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_EPTP, task->domain.curDomain->archInfo.ia32eInfo.cr3);

#if CONFIG_IA32E_VTX_FEATURE_VPID

    if (cpu->cpuFlags.fields.vpid != 0)
        __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VPID, task->domain.curDomain->archInfo.ia32eInfo.vpid);
    
#endif

    /* misc */

    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_ACTIVITY_STATE, IA32E_VTX_VMCS_GUEST_ACTIVE);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_VMCS_LINK_POINTER, UINT64_MAX);
}

void ia32eCpuVtxInit(void)
{
    /* validate that we are vcpu capable */

    ia32ePerCpu_t *cpu = NULL;
    ia32eGlobal_t *global = NULL;

    uint64_t featureCtrl = 0;

    uint64_t basic = 0;

    uint32_t ia32eVmxPinbasedCtls = 0;
    uint32_t ia32eVmxProcbasedCtls = 0;
    uint32_t ia32eVmxExitCtls = 0;
    uint32_t ia32eVmxEntryCtls = 0;

    uint64_t pinBasedCtls = 0;    
    uint64_t exitCtls = 0;
    uint64_t entryCtls = 0;
    uint64_t procbasedCtls = 0;

    uint64_t procbasedCtls2 = 0;

    uint64_t vpidCap = 0;

    uint32_t regs21[4] = {0};
    uint32_t regs22[4] = {0};

    uint32_t vmxMisc;

    uint32_t i = 0;

    uint64_t cr0 = 0;
    uint64_t cr4 = 0;

    cpu = ia32eThisCpuData();
    global = cpu->global;

    featureCtrl = __ia32eRdmsr(IA32E_FEATURE_CONTROL);

    if ((featureCtrl & IA32E_FEATURE_CONTROL_LOCKED_MASK) == 0) {

        featureCtrl |= IA32E_FEATURE_CONTROL_VMX_OUTSIDE_SMX_MASK;
        featureCtrl |= IA32E_FEATURE_CONTROL_LOCKED_MASK;

        __ia32eWrmsr(IA32E_FEATURE_CONTROL, featureCtrl);

    } else if ((featureCtrl & IA32E_FEATURE_CONTROL_VMX_OUTSIDE_SMX_MASK) == 0) {
        return;
    }

    basic = __ia32eRdmsr(IA32E_VMX_BASIC);
    
    if (!testBitLe(basic, IA32E_VMX_BASIC_INFO_REPORTS_BIT))
        return;

    if (testBitLe(basic, IA32E_VMX_BASIC_TRUE_CTLS_BIT)) {

        ia32eVmxPinbasedCtls = IA32E_VMX_TRUE_PINBASED_CTLS;
        ia32eVmxExitCtls = IA32E_VMX_TRUE_EXIT_CTLS;
        ia32eVmxEntryCtls = IA32E_VMX_TRUE_ENTRY_CTLS;
        ia32eVmxProcbasedCtls = IA32E_VMX_TRUE_PROCBASED_CTLS;

    } else {

        ia32eVmxPinbasedCtls = IA32E_VMX_PINBASED_CTLS;
        ia32eVmxExitCtls = IA32E_VMX_EXIT_CTLS;
        ia32eVmxEntryCtls = IA32E_VMX_ENTRY_CTLS;
        ia32eVmxProcbasedCtls = IA32E_VMX_PROCBASED_CTLS;
    }

    pinBasedCtls = __ia32eRdmsr(ia32eVmxPinbasedCtls);
    exitCtls = __ia32eRdmsr(ia32eVmxExitCtls);
    entryCtls = __ia32eRdmsr(ia32eVmxEntryCtls);
    procbasedCtls = __ia32eRdmsr(ia32eVmxProcbasedCtls);

    if (!testBitLe(pinBasedCtls, IA32E_VTX_VMCS_PINBASED_CTLS_EXTERNAL_INTERRUPT_EXITING_BIT + 32) ||
        !testBitLe(pinBasedCtls, IA32E_VTX_VMCS_PINBASED_CTLS_NMI_EXITING_BIT + 32) ||
        !testBitLe(pinBasedCtls, IA32E_VTX_VMCS_PINBASED_CTLS_VIRTUAL_NMIS_BIT + 32) || 
        !testBitLe(pinBasedCtls, IA32E_VTX_VMCS_PINBASED_CTLS_VMX_PREEMPTION_TIMER_BIT + 32) ||
    
        !testBitLe(exitCtls, IA32E_VTX_VMCS_EXIT_CTLS_ACKNOWLEDGE_INTERRUPT_ON_EXIT_BIT + 32) ||
        !testBitLe(exitCtls, IA32E_VTX_VMCS_EXIT_CTLS_HOST_ADDRESS_SPACE_SIZE_BIT + 32) ||
        (cpu->cpuFlags.fields.pat != 0 && !testBitLe(exitCtls, IA32E_VTX_VMCS_EXIT_CTLS_LOAD_PAT_BIT + 32)) ||
        (cpu->cpuFlags.fields.pat != 0 && !testBitLe(exitCtls, IA32E_VTX_VMCS_EXIT_CTLS_SAVE_PAT_BIT + 32)) ||
        !testBitLe(exitCtls, IA32E_VTX_VMCS_EXIT_CTLS_LOAD_EFER_BIT + 32) || 
        !testBitLe(exitCtls, IA32E_VTX_VMCS_EXIT_CTLS_SAVE_EFER_BIT + 32) ||
        !testBitLe(exitCtls, IA32E_VTX_VMCS_EXIT_CTLS_SAVE_VMX_PREEMPTION_TIMER_BIT + 32) ||
        
        (cpu->cpuFlags.fields.pat != 0 && !testBitLe(entryCtls, IA32E_VTX_VMCS_ENTRY_CTLS_LOAD_PAT_BIT + 32)) ||
        !testBitLe(entryCtls, IA32E_VTX_VMCS_ENTRY_CTLS_LOAD_EFER_BIT + 32) ||
    
        !testBitLe(procbasedCtls, IA32E_VTX_VMCS_PROCBASED_CTLS_INTERRUPT_WINDOW_EXITING_BIT + 32) ||
        !testBitLe(procbasedCtls, IA32E_VTX_VMCS_PROCBASED_CTLS_RDPMC_EXITING_BIT + 32) ||

        (!testBitLe(procbasedCtls, IA32E_VTX_VMCS_PROCBASED_CTLS_RDTSC_EXITING_BIT + 32) && 
          CONFIG_IA32E_VTX_TSD) ||

        !testBitLe(procbasedCtls, IA32E_VTX_VMCS_PROCBASED_CTLS_NMI_WINDOW_EXITING_BIT  + 32) || 
        !testBitLe(procbasedCtls, IA32E_VTX_VMCS_PROCBASED_CTLS_IO_BITMAPS_BIT + 32) || 
        !testBitLe(procbasedCtls, IA32E_VTX_VMCS_PROCBASED_CTLS_MSR_BITMAPS_BIT + 32) ||
        !testBitLe(procbasedCtls, IA32E_VTX_VMCS_PROCBASED_CTLS_SECONDARY_CTLS_BIT + 32)) {

        return;   
    }

    if (cpu->cpuFlags.fields.monitorMwait != 0 && 
        (!testBitLe(procbasedCtls, IA32E_VTX_VMCS_PROCBASED_CTLS_MWAIT_EXITING_BIT + 32) || 
         !testBitLe(procbasedCtls, IA32E_VTX_VMCS_PROCBASED_CTLS_MONITOR_EXITING_BIT + 32))) {

        return;
    }

    procbasedCtls2 = __ia32eRdmsr(IA32E_VMX_PROCBASED_CTLS2);
    if (!testBitLe(procbasedCtls2, IA32E_VTX_VMCS_PROCBASED_CTLS2_EPT_BIT + 32) ||
        !testBitLe(procbasedCtls2, IA32E_VTX_VMCS_PROCBASED_CTLS2_URG_BIT + 32) ||
        !testBitLe(procbasedCtls2, IA32E_VTX_VMCS_PROCBASED_CTLS2_WBINVD_EXITING_BIT + 32) ||
        
        (cpu->cpuFlags.fields.sgx != 0 && 
         !testBitLe(procbasedCtls2, IA32E_VTX_VMCS_PROCBASED_CTLS2_ENCLS_EXITING_BIT + 32))) {

        return;
    } 

    vpidCap = __ia32eRdmsr(IA32E_VMX_EPT_VPID_CAP);
    if ((vpidCap & IA32E_VMX_EPT_VPID_CAP_PWLEN4_MASK) == 0 || (vpidCap & IA32E_VMX_EPT_VPID_CAP_PAGE_WB_MASK) == 0)
        return;

    cpu->cpuFlags.fields.vpid = testBitLe(procbasedCtls2, IA32E_VTX_VMCS_PROCBASED_CTLS2_VPID_BIT + 32);

    cpu->cpuFlags.fields.ept2mb = (vpidCap & IA32E_VMX_EPT_VPID_CAP_PAGE_2MB_MASK) != 0;
    cpu->cpuFlags.fields.ept1gb = (vpidCap & IA32E_VMX_EPT_VPID_CAP_PAGE_1GB_MASK) != 0;
    cpu->cpuFlags.fields.eptUc = (vpidCap & IA32E_VMX_EPT_VPID_CAP_PAGE_UC_MASK) != 0;
    cpu->cpuFlags.fields.eptAd = (vpidCap & IA32E_VMX_EPT_VPID_CAP_PAGE_AD_MASK) != 0;

    cpu->vtx.revisionId = basic & (0xffffffff & ~(1 << 31));
    cpu->vtx.ia32eVmxPinbasedCtls = ia32eVmxPinbasedCtls;
    cpu->vtx.ia32eVmxEntryCtls = ia32eVmxEntryCtls;
    cpu->vtx.ia32eVmxExitCtls = ia32eVmxExitCtls;
    cpu->vtx.ia32eVmxProcbasedCtls = ia32eVmxProcbasedCtls;

    /* setup per host cpu state */

    ia32eCpuid(21, 0, &regs21[0], &regs21[1], &regs21[2], &regs21[3]);

    if (regs21[0] == 0 || regs21[1] == 0 || regs21[2] == 0) {

        ia32eCpuid(22, 0, &regs22[0], &regs22[1], &regs22[2], &regs22[3]);

        if ((regs22[0] & 0xffff) == 0)
            cpu->vtx.tscFrequencyHz = ia32eVtxCalibrateTscManual();
        else
            cpu->vtx.tscFrequencyHz = (regs22[0] & 0xffff) * 1000000;

    } else {
        cpu->vtx.tscFrequencyHz = regs21[2] * (regs21[1] / regs21[0]);
    }

    vmxMisc = __ia32eRdmsr(IA32E_VMX_MISC);
    cpu->vtx.vmxPreemptFrequencyHz = cpu->vtx.tscFrequencyHz / (1 << (vmxMisc & 0x1f));

    cpu->vtx.hostDr1 = __ia32eReadDr1();
    cpu->vtx.hostDr2 = __ia32eReadDr2();
    cpu->vtx.hostDr3 = __ia32eReadDr3();
    cpu->vtx.hostDr6 = __ia32eReadDr6();
    cpu->vtx.hostDr7 = __ia32eReadDr7();

    STATIC_ASSERT(ARRAY_LEN(cpu->vtx.areas.vmexitLoadArea) == ARRAY_LEN(ia32eVtxVmcsMsrCtxList));
    STATIC_ASSERT(ARRAY_LEN(cpu->vtx.areas.vmexitStoreVmentryLoadArea) == ARRAY_LEN(ia32eVtxVmcsMsrCtxList));

    cpu->vtx.areas.msrAreaCount = ARRAY_LEN(ia32eVtxVmcsMsrCtxList);
    
    for (i = 0; i < ARRAY_LEN(ia32eVtxVmcsMsrCtxList); i++) {

        cpu->vtx.areas.vmexitLoadArea[i].msrIndex = ia32eVtxVmcsMsrCtxList[i];
        cpu->vtx.areas.vmexitLoadArea[i].msrData = __ia32eRdmsr(ia32eVtxVmcsMsrCtxList[i]);
        cpu->vtx.areas.vmexitStoreVmentryLoadArea[i].msrIndex = ia32eVtxVmcsMsrCtxList[i];    
    }

    cpu->vtx.areas.vmxonRegion.header = cpu->vtx.revisionId;

    /* try finalise vmx operation and check if vmx is a security risk */

    cr0 = (__ia32eReadCr0() | __ia32eRdmsr(IA32E_VMX_CR0_FIXED0)) & __ia32eRdmsr(IA32E_VMX_CR0_FIXED1);
    cr4 = (__ia32eReadCr4() | __ia32eRdmsr(IA32E_VMX_CR4_FIXED0)) & __ia32eRdmsr(IA32E_VMX_CR4_FIXED1);

    /* security checks */

    if (((cr4 & IA32E_CR4_TSD_MASK) != 0) != CONFIG_IA32E_TSD ||
        (cr4 & IA32E_CR4_MCE_MASK) != 0 || 
        (cr4 & IA32E_CR4_PCE_MASK) != 0 ||
        (cr4 & IA32E_CR4_LA57_MASK) != 0 ||
        (cr4 & IA32E_CR4_VMXE_MASK) == 0 ||
        (cr4 & IA32E_CR4_OSXSAVE_MASK) != 0 ||
        (cr4 & IA32E_CR4_PKE_MASK) != 0 ||
        (cr4 & IA32E_CR4_UINTR_MASK) != 0 ||

        (!CONFIG_IA32E_FEATURE_PCID && ((cr4 & IA32E_CR4_PCIDE_MASK) != 0)) ||
        (!CONFIG_IA32E_FEATURE_SMEP && ((cr4 & IA32E_CR4_SMEP_MASK) != 0)) ||
        (!CONFIG_IA32E_FEATURE_SMAP && ((cr4 & IA32E_CR4_SMAP_MASK) != 0)) ||
        (!CONFIG_IA32E_FEATURE_UMIP && ((cr4 & IA32E_CR4_UMIP_MASK) != 0))){

        return;
    }

    /* features we can afford to lose */

    if ((cr4 & IA32E_CR4_DE_MASK) == 0)
        cpu->cpuFlags.fields.de = 0;

    if ((cr4 & IA32E_CR4_PCIDE_MASK) == 0)
        cpu->cpuFlags.fields.pcid = 0;

    if ((cr4 & IA32E_CR4_FSGSBASE_MASK) == 0)
        cpu->cpuFlags.fields.fsgsbase = 0;

    if ((cr4 & IA32E_CR4_SMEP_MASK) == 0)
        cpu->cpuFlags.fields.smep = 0;

    if ((cr4 & IA32E_CR4_SMAP_MASK) == 0)
        cpu->cpuFlags.fields.smap = 0;

    __ia32eWriteCr0(cr0);
    __ia32eWriteCr4(cr4);

    if (!__ia32eVmxon(ia32eVirtToPhysStatic(&cpu->vtx.areas.vmxonRegion)))
        return;

    /* we are vcpu capable */

    cpu->cpuFlags.fields.vcpuCapable = 1;
    global->gFlags.fields.vcpuCapableExists = 1;
    global->gFlags.fields.vpidCapableExists |= cpu->cpuFlags.fields.vpid;
}

void ia32eGlobalVtxInit(void)
{
    ia32eGlobal_t *global = NULL;
    uint32_t i = 0;

    global = ia32eThisCpuData()->global;

    if (global->gFlags.fields.vcpuCapableExists == 0)
        return;

#if CONFIG_IA32E_VTX_FEATURE_VPID
    global->vtxGlobal.vpidCtr = 1;
#endif

    memset(global->vtxGlobal.msrBitmap, 0xff, sizeof(global->vtxGlobal.msrBitmap));

    for (i = 0; i < ARRAY_LEN(ia32eVtxVmcsMsrAllowList); i++)
        ia32eVtxVmcsUntrapMsr(global->vtxGlobal.msrBitmap, ia32eVtxVmcsMsrAllowList[i]); 
    
    global->gFlags.fields.allEptAd = 1;

    K_DYNAMIC_ASSERT(global->numCpus <= ARRAY_LEN(global->cpuTable));

    for (i = 0; i < global->numCpus; i++) {

        if (global->cpuTable[i].cpuFlags.fields.online != 0 && global->cpuTable[i].cpuFlags.fields.eptAd == 0) {
            global->gFlags.fields.allEptAd = 0;
            break;
        }
    }
}

void ia32eCpuVtxTaskInit(kSchedTask_t *task)
{
    task->ctx.ia32eCtx.fpCtx.fcw = IA32E_DEFAULT_FCW;
    task->ctx.ia32eCtx.fpCtx.mxcsr = IA32E_DEFAULT_MXCSR;
    task->ctx.ia32eCtx.fpCtx.mxcsrMask = ia32eThisCpuData()->mxcsrMask;

    task->ctx.ia32eCtx.rflags |= IA32E_FLAGS_IF_MASK;

    task->ctx.ia32eCtx.rip = (uintptr_t)__ia32eVmlaunchStub;

    task->ctx.ia32eCtx.cs = IA32E_KCS_SELECTOR;
    task->ctx.ia32eCtx.ss = IA32E_KDS_SELECTOR;

    task->ctx.ia32eCtx.dr0 = (uintptr_t)task->ctx.ia32eCtx.kStack.padding;
    task->ctx.ia32eCtx.ksp = (uint64_t)&task->ctx.ia32eCtx.kStack.stack[sizeof(task->ctx.ia32eCtx.kStack.stack)];   

    task->ctx.ia32eCtx.rsp = task->ctx.ia32eCtx.ksp;
}

void ia32eCpuVtxTaskSaveCtx(kSchedTask_t *task)
{
    ia32ePerCpu_t *cpu = NULL;
    uint32_t i = 0;

    cpu = ia32eThisCpuData();

    STATIC_ASSERT(ARRAY_LEN(cpu->vtx.areas.vmexitStoreVmentryLoadArea) == 
                  ARRAY_LEN(task->ctx.ia32eCtx.vtx.vmexitStoreVmentryLoadAreaData));

    for (i = 0; i < ARRAY_LEN(task->ctx.ia32eCtx.vtx.vmexitStoreVmentryLoadAreaData); i++) 
        task->ctx.ia32eCtx.vtx.vmexitStoreVmentryLoadAreaData[i] = cpu->vtx.areas.vmexitStoreVmentryLoadArea[i].msrData;
}

void ia32eCpuVtxTaskRestoreCtx(kSchedTask_t *task)
{
    ia32ePerCpu_t *cpu = NULL;

    kSchedThread_t *thread = NULL;
    kSchedLsr_t *lsr = NULL;
    uintptr_t vmcsPhys = 0;

    uint32_t i = 0;

    cpu = ia32eThisCpuData();

    switch (task->taggedInfo.type) {
    
        case K_TASK_THREAD:
            thread = &task->taggedInfo.info.thread;
            vmcsPhys = thread->archInfo.ia32eInfo.vtxInfo.vtxParam.vmcsPhys;
            break;

        case K_TASK_LSR:
            lsr = &task->taggedInfo.info.lsr;
            vmcsPhys = lsr->archInfo.ia32eInfo.vtxInfo.vtxParam.vmcsPhys;
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }

    STATIC_ASSERT(ARRAY_LEN(cpu->vtx.areas.vmexitStoreVmentryLoadArea) == 
                  ARRAY_LEN(task->ctx.ia32eCtx.vtx.vmexitStoreVmentryLoadAreaData));

    for (i = 0; i < ARRAY_LEN(cpu->vtx.areas.vmexitStoreVmentryLoadArea); i++) 
        cpu->vtx.areas.vmexitStoreVmentryLoadArea[i].msrData = task->ctx.ia32eCtx.vtx.vmexitStoreVmentryLoadAreaData[i];

    if (!task->ctx.ia32eCtx.vtx.vmcsInitialized) {
        ia32eCpuVtxVmcsSetup(task);
        task->ctx.ia32eCtx.vtx.vmcsInitialized = true;
        return;
    }

#if CONFIG_KDYNAMIC_ASSERT
    K_DYNAMIC_ASSERT(__ia32eVmptrld(vmcsPhys));
#else 
    __ia32eVmptrld(vmcsPhys);
#endif 
}

void ia32eCpuVtxEnterDomain(kDomain_t *domain)
{
    ia32ePerCpu_t *cpu = NULL;

    cpu = ia32eThisCpuData();

    STATIC_ASSERT(sizeof(cpu->vtx.areas.ioBitmap) == sizeof(domain->archInfo.ia32eInfo.iopb));

    memcpy(cpu->vtx.areas.ioBitmap, domain->archInfo.ia32eInfo.iopb, sizeof(domain->archInfo.ia32eInfo.iopb));
}

bool ia32eCpuVtxThreadParamIsVm(archSchedThreadParam_t *param)
{
    kPluginTaskThreadParam_t *threadParam = NULL;
    kDomain_t *domain = NULL;

    threadParam = containerOf(param, kPluginTaskThreadParam_t, archParam);
    domain = kDomainUniverseGet(threadParam->domId);

    K_DYNAMIC_ASSERT(domain);

    return domain->archInfo.ia32eInfo.vm;
}

bool ia32eCpuVtxLsrParamIsVm(archSchedLsrParam_t *param)
{
    kPluginTaskLsrParam_t *lsrParam = NULL;
    kDomain_t *domain = NULL;

    lsrParam = containerOf(param, kPluginTaskLsrParam_t, archParam);
    domain = kDomainUniverseGet(lsrParam->domId);

    K_DYNAMIC_ASSERT(domain);

    return domain->archInfo.ia32eInfo.vm;   
}

int ia32eCpuVtxThreadInfoInit(archSchedThreadInfo_t *info, archSchedThreadParam_t *param)
{
    kPluginTaskThreadParam_t *threadParam = NULL;

    threadParam = containerOf(param, kPluginTaskThreadParam_t, archParam);

    if (!ia32eCpuVtxIsVcpuCapable(threadParam->cpuId))
        return -EINVAL;

    return ia32eCpuVtxTaskInfoInit(&info->ia32eInfo.vtxInfo, threadParam->domId, &param->ia32eParam.vtxParam);
}

int ia32eCpuVtxLsrInfoInit(archSchedLsrInfo_t *info, archSchedLsrParam_t *param)
{
    kPluginTaskLsrParam_t *lsrParam = NULL;

    lsrParam = containerOf(param, kPluginTaskLsrParam_t, archParam);

    if (!ia32eCpuVtxIsVcpuCapable(lsrParam->cpuId))
        return -EINVAL;    

    return ia32eCpuVtxTaskInfoInit(&info->ia32eInfo.vtxInfo, lsrParam->domId, &param->ia32eParam.vtxParam);    
}

int ia32eCpuVtxDomainInfoInit(archDomainInfo_t *info, archDomainParam_t *param)
{
    ia32eGlobal_t *global = NULL;

    kPluginDomainParam_t *domainParam = NULL;
    uintptr_t pml4Phys = 0;
    uintptr_t pml4Virt = 0;

    uint32_t i = 0;

    global = ia32eThisCpuData()->global;

    if (global->gFlags.fields.vcpuCapableExists == 0)
        return -EINVAL;

    domainParam = containerOf(param, kPluginDomainParam_t, archParam);
    pml4Phys = param->ia32eParam.pml4BasePhys;
    pml4Virt = (uintptr_t)param->ia32eParam.pml4BaseVirt;

    /* Vm domains should NOT be allowed to partake in IPC, and must have a valid _start to boot in realmode */

    for (i = 0; i < ARRAY_LEN(domainParam->param.invocationInfo.invokePermMap); i++) {
        if (domainParam->param.invocationInfo.invokePermMap[i] != 0)
            return -EINVAL;
    }

    if (domainParam->param.invocationInfo.invocationIpc.valid || 
        domainParam->param.invocationInfo._start > UINT16_MAX ||
        (pml4Phys & 0xfff) != 0 || (pml4Virt & 0xfff) != 0) {

        return -EINVAL;
    }

    STATIC_ASSERT(sizeof(info->ia32eInfo.iopb) == sizeof(param->ia32eParam.iopb));

    memcpy(info->ia32eInfo.iopb, param->ia32eParam.iopb, sizeof(info->ia32eInfo.iopb));

    info->ia32eInfo.cr3 = pml4Phys | (global->gFlags.fields.allEptAd << 6) | (3 << 3) | IA32E_VTX_EPT_WB;
    info->ia32eInfo.vm = param->ia32eParam.vm;

#if CONFIG_IA32E_VTX_FEATURE_VPID

    K_DYNAMIC_ASSERT(global->vtxGlobal.vpidCtr <= UINT16_MAX);

    if (global->gFlags.fields.vpidCapableExists != 0) {
        info->ia32eInfo.vpid = global->vtxGlobal.vpidCtr;
        global->vtxGlobal.vpidCtr++;
    }
    
#endif

    info->ia32eInfo.numVcpus = 0;
    dqInit(&info->ia32eInfo.vcpuVector);

    return 0;
}

#endif