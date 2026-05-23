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

#include <ia32eVmcsSetup.h>

#if CONFIG_IA32E_VTX

#include <ia32eCpu.h>
#include <ia32eVma.h>
#include <export/kDbgInterface.h>

extern 
void __ia32eVmexitStub(void);

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

void ia32eVtxVmcsSetup(kSchedTask_t *task)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eGlobal_t *global = NULL;

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

    /* vmx mode */

    task->ctx.ia32eCtx.vtx.vmcsVirt->header = cpu->vtx.revisionId;

#if CONFIG_KDYNAMIC_ASSERT

    K_DYNAMIC_ASSERT(__ia32eVmclear(task->ctx.ia32eCtx.vtx.vmcsPhys));
    K_DYNAMIC_ASSERT(__ia32eVmptrld(task->ctx.ia32eCtx.vtx.vmcsPhys));

#else 

    __ia32eVmclear(task->ctx.ia32eCtx.vtx.vmcsPhys);
    __ia32eVmptrld(task->ctx.ia32eCtx.vtx.vmcsPhys);

#endif

    /* controls */

    exceptionBitmap = IA32E_VTX_VMCS_EXCEPTION_BITMAP_DEBUG_MASK | 
                      IA32E_VTX_VMCS_EXCEPTION_BITMAP_ALIGNMENT_CHECK_MASK;

    pin = (1U << IA32E_VTX_VMCS_PINBASED_CTLS_EXTERNAL_INTERRUPT_EXITING_BIT) |
                (1U << IA32E_VTX_VMCS_PINBASED_CTLS_NMI_EXITING_BIT) |
                (1U << IA32E_VTX_VMCS_PINBASED_CTLS_VIRTUAL_NMIS_BIT);
            
    proc = (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_MWAIT_EXITING_BIT) |
                (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_MONITOR_EXITING_BIT) |
                (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_IO_BITMAPS_BIT) |
                (1U << IA32E_VTX_VMCS_PROCBASED_CTLS_MSR_BITMAPS_BIT) |
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

    if (cpu->cpuFlags.fields.vme)
        cr4Mask |= IA32E_CR4_VME_MASK | IA32E_CR4_PVI_MASK;

    if (cpu->cpuFlags.fields.umip)
        cr4Mask |= IA32E_CR4_UMIP_MASK;

    if (cpu->cpuFlags.fields.fsgsbase)
        cr4Mask |= IA32E_CR4_FSGSBASE_MASK;

    if (cpu->cpuFlags.fields.pcid)
        cr4Mask |= IA32E_CR4_PCIDE_MASK;

    if (cpu->cpuFlags.fields.smep)
        cr4Mask |= IA32E_CR4_SMEP_MASK;

    if (cpu->cpuFlags.fields.smap)
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

    if (cpu->cpuFlags.fields.vpid != 0)
        __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VPID, task->domain.curDomain->archInfo.ia32eInfo.vpid);

    /* misc */

    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_ACTIVITY_STATE, IA32E_VTX_VMCS_GUEST_ACTIVE);
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_VMCS_LINK_POINTER, UINT64_MAX);
}

#endif