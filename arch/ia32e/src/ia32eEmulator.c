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

static 
ia32eEmulatorFn_t ia32eEmulatorDispatchTable[] = {

};

void ia32eEmulatorFailure(void)
{

}

void ia32eEmulatorDispatcher(ia32eVmexitRegs_t *regs)
{

}

#endif