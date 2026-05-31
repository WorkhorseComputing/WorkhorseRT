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

#include <ia32eEmulator.h>

#if CONFIG_IA32E_VTX

#include <ia32eCpu.h>
#include <import/kSyscallHandler.h>
#include <workhorse/kTick/kTick.h>

char *ia32eEmulatorErrorTable[] = {
    [0] = "UNKNOWN, ERRCODE 0",
    [1] = "VMCALL executed in VMX root operation",
    [2] = "VMCLEAR with invalid physical address",
    [3] = "VMCLEAR with VMXON pointer",
    [4] = "VMLAUNCH with non-clear VMCS",
    [5] = "VMRESUME with non-launched VMCS",
    [6] = "VMRESUME after VMXOFF",
    [7] = "VM entry with invalid control field(s)",
    [8] = "VM entry with invalid host-state field(s)",
    [9] = "VMPTRLD with invalid physical address",
    [10] = "VMPTRLD with VMXON pointer",
    [11] = "VMPTRLD with incorrect VMCS revision identifier",
    [12] = "VMREAD / VMWRITE from / to unsupported VMCS component",
    [14] = "UNKNOWN, ERRCODE 14",
    [13] = "VMWRITE to read-only VMCS component",
    [15] = "VMXON executed in VMX root operation",
    [16] = "VM entry with invalid executive-VMCS pointer",
    [17] = "VM entry with non-launched executive VMCS",
    [18] = "VM entry with executive-VMCS pointer not VMXON pointer",
    [19] = "VMCALL with non-clear VMCS (dual-monitor treatment)",
    [20] = "VMCALL with invalid VM-exit control fields",
    [22] = "VMCALL with incorrect MSEG revision identifier",
    [23] = "VMXOFF under dual-monitor treatment of SMIs and SMM",
    [24] = "VMCALL with invalid SMM-monitor features (dual-monitor treatment)",
    [25] = "VM entry with invalid VM-exec control fields in executive VMCS",
    [26] = "VM entry with events blocked by MOV SS",
    [27] = "UNKNOWN, ERRCODE 27",
    [28] = "Invalid operand to INVEPT / INVVPID"
};

/* General purpose events */

static 
void ia32eEmulatorUd(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}

static 
void ia32eEmulatorNop(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}


static 
void ia32eEmulatorNopAdvance(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
    
}

ATTR_NORETURN
static
void ia32eEmulatorVcpuFailure(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
    kSyscallHandler(WORKHORSE_SYS_SCHED_CTRL, WORKHORSE_SCHED_CTRL_FAILURE, 0);
    UNREACHABLE();
}

static 
void ia32eEmulatorAccessDenied(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
    
}

/* Specific events */

static 
void ia32eEmulatorException(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}

static 
void ia32eEmulatorExtIntr(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}

static 
void ia32eEmulatorCpuid(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}

static 
void ia32eEmulatorVmcall(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}

static 
void ia32eEmulatorCrAccess(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}

static 
void ia32eEmulatorRdmsr(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}

static 
void ia32eEmulatorWrmsr(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}

static 
void ia32eEmulatorMceDuringVmentry(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}

static 
void ia32eEmulatorVmxPreempt(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{

}

/* Emulator entry */

static 
ia32eEmulatorFn_t ia32eEmulatorDispatchTable[] = {
    [IA32E_VTX_EXIT_REASON_EXCEPTION] = ia32eEmulatorException,
    [IA32E_VTX_EXIT_REASON_EXT_INTR] = ia32eEmulatorExtIntr,
    [IA32E_VTX_EXIT_REASON_TRIPLE_FAULT] = ia32eEmulatorVcpuFailure,
    [IA32E_VTX_EXIT_REASON_INIT] = ia32eEmulatorNop,
    [IA32E_VTX_EXIT_REASON_SIPI] = ia32eEmulatorNop,
    [IA32E_VTX_EXIT_REASON_INTR_WINDOW] = ia32eEmulatorNop,
    [IA32E_VTX_EXIT_REASON_NMI_WINDOW] = ia32eEmulatorNop,
    [IA32E_VTX_EXIT_REASON_TASK_SWITCH] = ia32eEmulatorVcpuFailure,
    [IA32E_VTX_EXIT_REASON_CPUID] = ia32eEmulatorCpuid,
    [IA32E_VTX_EXIT_REASON_GETSEC] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_INVD] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_RDPMC] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_RDTSC] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_VMCALL] = ia32eEmulatorVmcall,
    [IA32E_VTX_EXIT_REASON_VMCLEAR] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_VMLAUNCH] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_VMPTRLD] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_VMPTRST] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_VMREAD] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_VMRESUME] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_VMWRITE] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_VMXOFF] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_VMXON] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_CR_ACCESS] = ia32eEmulatorCrAccess,
    [IA32E_VTX_EXIT_REASON_INOUT] = ia32eEmulatorAccessDenied,
    [IA32E_VTX_EXIT_REASON_RDMSR] = ia32eEmulatorRdmsr,
    [IA32E_VTX_EXIT_REASON_WRMSR] = ia32eEmulatorWrmsr,
    [IA32E_VTX_EXIT_REASON_MWAIT] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_MONITOR] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_MCE_DURING_ENTRY] = ia32eEmulatorMceDuringVmentry,
    [IA32E_VTX_EXIT_REASON_EPT_FAULT] = ia32eEmulatorAccessDenied,
    [IA32E_VTX_EXIT_REASON_EPT_MISCONFIG] = ia32eEmulatorVcpuFailure,
    [IA32E_VTX_EXIT_REASON_INVEPT] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_VMX_PREEMPT] = ia32eEmulatorVmxPreempt,
    [IA32E_VTX_EXIT_REASON_INVVPID] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_WBINVD] = ia32eEmulatorNopAdvance,
    [IA32E_VTX_EXIT_REASON_XSETBV] = ia32eEmulatorUd,
    [IA32E_VTX_EXIT_REASON_ENCLS] = ia32eEmulatorUd,
};

static
void ia32eEmulatorEntryHandler(void)
{
    ia32ePerCpu_t *cpu = NULL;
    kSchedTask_t *task = NULL;

    cpu = ia32eThisCpuData();
    task = kTickGetRunningTask();
    
    __ia32eWriteDr0(task->ctx.ia32eCtx.dr0);
    __ia32eWriteDr1(cpu->vtx.hostDr1);
    __ia32eWriteDr2(cpu->vtx.hostDr2);
    __ia32eWriteDr3(cpu->vtx.hostDr3);
    __ia32eWriteDr6(cpu->vtx.hostDr6);
    __ia32eWriteDr7(cpu->vtx.hostDr7);
}


ATTR_NORETURN
void ia32eEmulatorVcpuFailureEntry(void)
{
    ia32eEmulatorEntryHandler();
    cpuEnableInterrupts();

    ia32eEmulatorVcpuFailure(NULL);

    UNREACHABLE();
}

void ia32eEmulatorDispatcher(ia32eVmexitRegs_t *regs)
{
    ia32eEmulatorEntryHandler();
}

#endif