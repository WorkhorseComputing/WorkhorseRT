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

#include <ia32eHostCpu.h>
#include <ia32eCpu.h>
#include <ia32eHpet.h>
#include <ia32eVma.h>
#include <lib/acpi.h>
#include <export/kDbgInterface.h>

#if CONFIG_IA32E_VTX

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
    global->gFlags.fields.pcidCapableExists |= cpu->cpuFlags.fields.vcpuCapable;
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
}

#endif