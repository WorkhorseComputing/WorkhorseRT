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

#include <ia32eVmcs.h>
#include <ia32eCpu.h>
#include <export/kDbgInterface.h>

#if CONFIG_IA32E_VTX

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

    uint32_t exceptionBitmap = 0;
    uint32_t pin = 0;
    uint32_t proc = 0;
    uint32_t exit = 0;
    uint32_t entry = 0;
    uint32_t proc2 = 0;

    cpu = ia32eThisCpuData();

    /* vmx mode */

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

    /* cpu state */
}

#endif