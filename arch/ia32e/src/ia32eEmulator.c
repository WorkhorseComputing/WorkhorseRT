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
#include <export/kDbgInterface.h>
#include <import/kSyscallHandler.h>
#include <workhorse/kTick/kTick.h>
#include <stdatomic.h>

#define IA32E_EMULATOR_CPUID1_C_TARGET_MASK                                             \
  (IA32E_CPUID1_C_SSE3_MASK | IA32E_CPUID1_C_PCLMULQDQ_MASK |                           \
   IA32E_CPUID1_C_SSSE3_MASK | IA32E_CPUID1_C_FMA_MASK |                                \
   IA32E_CPUID1_C_CX16_MASK | IA32E_CPUID1_C_PCID_MASK |                                \
   IA32E_CPUID1_C_SSE4_1_MASK | IA32E_CPUID1_C_SSE4_2_MASK |                            \
   IA32E_CPUID1_C_MOVBE_MASK | IA32E_CPUID1_C_POPCNT_MASK |                             \
   IA32E_CPUID1_C_AES_NI_MASK | IA32E_CPUID1_C_F16C_MASK |                              \
   IA32E_CPUID1_C_RDRAND_MASK)

#define IA32E_EMULATOR_CPUID1_D_TARGET_MASK                                             \
  (IA32E_CPUID1_D_FPU_MASK | IA32E_CPUID1_D_VME_MASK | IA32E_CPUID1_D_DE_MASK |         \
   IA32E_CPUID1_D_PSE_MASK | IA32E_CPUID1_D_TSC_MASK | IA32E_CPUID1_D_MSR_MASK |        \
   IA32E_CPUID1_D_PAE_MASK | IA32E_CPUID1_D_CX8_MASK | IA32E_CPUID1_D_SEP_MASK |        \
   IA32E_CPUID1_D_PGE_MASK | IA32E_CPUID1_D_CMOV_MASK | IA32E_CPUID1_D_PAT_MASK |       \
   IA32E_CPUID1_D_PSE36_MASK | IA32E_CPUID1_D_CLFLUSH_MASK | IA32E_CPUID1_D_MMX_MASK |  \
   IA32E_CPUID1_D_FXSR_MASK | IA32E_CPUID1_D_SSE_MASK | IA32E_CPUID1_D_SSE2_MASK |      \
   IA32E_CPUID1_D_SS_MASK)

#define IA32E_EMULATOR_CPUID7_0_B_TARGET_MASK                                           \
  (IA32E_CPUID7_0_B_FSGSBASE_MASK | IA32E_CPUID7_0_B_BMI1_MASK |                        \
   IA32E_CPUID7_0_B_HLE_MASK | IA32E_CPUID7_0_B_FDP_EXCEPTION_ONLY_MASK |               \
   IA32E_CPUID7_0_B_SMEP_MASK | IA32E_CPUID7_0_B_BMI2_MASK |                            \
   IA32E_CPUID7_0_B_ERMS_MASK | IA32E_CPUID7_0_B_INVPCID_MASK |                         \
   IA32E_CPUID7_0_B_RTM_MASK | IA32E_CPUID7_0_B_FCS_FDS_DEPR_MASK |                     \
   IA32E_CPUID7_0_B_RDSEED_MASK | IA32E_CPUID7_0_B_ADX_MASK |                           \
   IA32E_CPUID7_0_B_SMAP_MASK | IA32E_CPUID7_0_B_PCOMMIT_MASK |                         \
   IA32E_CPUID7_0_B_CLFLUSHOPT_MASK | IA32E_CPUID7_0_B_CLWB_MASK |                      \
   IA32E_CPUID7_0_B_SHA_MASK)

#define IA32E_EMULATOR_CPUID7_0_C_TARGET_MASK                                           \
  (IA32E_CPUID7_0_C_REFETCHWT1_MASK | IA32E_CPUID7_0_C_UMIP_MASK |                      \
   IA32E_CPUID7_0_C_GFNI_MASK | IA32E_CPUID7_0_C_VAES_MASK |                            \
   IA32E_CPUID7_0_C_VPCLMULQDQ_MASK | IA32E_CPUID7_0_C_AVX512_VNNI_MASK |               \
   IA32E_CPUID7_0_C_AVX512_BITALG_MASK | IA32E_CPUID7_0_C_AVX512_VPOPCNTDQ_MASK |       \
   IA32E_CPUID7_0_C_CLDEMOTE_MASK | IA32E_CPUID7_0_C_MOVDIRI_MASK |                     \
   IA32E_CPUID7_0_C_MOVDIR64B_MASK)

#define IA32E_EMULATOR_CPUID7_0_D_TARGET_MASK                                           \
  (IA32E_CPUID7_0_D_FSRM_MASK | IA32E_CPUID7_0_D_AVX512_VP2INTERSECT_MASK |             \
    IA32E_CPUID7_0_D_RTM_ABORT_MASK | IA32E_CPUID7_0_D_SERIALIZE_MASK |                 \
    IA32E_CPUID7_0_D_TSXLDTRK_MASK | IA32E_CPUID7_0_D_AVX512_FP16_MASK)

#define IA32E_EMULATOR_CPUID7_1_A_TARGET_MASK                                           \
  (IA32E_CPUID7_1_A_SM3_MASK | IA32E_CPUID7_1_A_SM4_MASK |                              \
   IA32E_CPUID7_1_A_RAO_INT_MASK | IA32E_CPUID7_1_A_AVX_VNNI_MASK |                     \
   IA32E_CPUID7_1_A_AVX512_BF16_MASK | IA32E_CPUID7_1_A_CMPCCXADD_MASK |                \
   IA32E_CPUID7_1_A_FZRM_MASK | IA32E_CPUID7_1_A_FSRS_MASK |                            \
   IA32E_CPUID7_1_A_RSRCS_MASK | IA32E_CPUID7_1_A_LKGS_MASK |                           \
   IA32E_CPUID7_1_A_WRMSRNS_MASK | IA32E_CPUID7_1_A_AVX_IFMA_MASK |                     \
   IA32E_CPUID7_1_A_BIOS_DONE_MASK | IA32E_CPUID7_1_A_MOVRS_MASK)

#define IA32E_EMULATOR_CPUID7_1_D_TARGET_MASK                                           \
  (IA32E_CPUID7_1_D_AVX512_VNNI_FP16_MASK | IA32E_CPUID7_1_D_AVX512_VNNI_INT8_MASK |    \
   IA32E_CPUID7_1_D_AVX512_NE_CONVERT_MASK | IA32E_CPUID7_1_D_AVX_VNNI_INT8_MASK |      \
   IA32E_CPUID7_1_D_AVX_NE_CONVERT_MASK | IA32E_CPUID7_1_D_AVX_VNNI_INT16_MASK |        \
   IA32E_CPUID7_1_D_AVX512_VNNI_INT16_MASK | IA32E_CPUID7_1_D_PREFETCHI_MASK |          \
   IA32E_CPUID7_1_D_AVX512_BF16_NE_MASK | IA32E_CPUID7_1_D_AVX10_MASK)

#define IA32E_EMULATOR_DB_DR6_TARGET_MASK                                               \
    (IA32E_DR6_BP0_MASK | IA32E_DR6_BP1_MASK | IA32E_DR6_BP2_MASK |                     \
     IA32E_DR6_BP3_MASK | IA32E_DR6_BD_MASK | IA32E_DR6_BS_MASK) 

#define IA32E_EMULATOR_X2APIC_LVT_TIMER_WRITE_RESERVED_MASK                                   \
    ((0xffULL << 8) | 0xfffffffffffc0000ULL)

#define ia32eEmulatorRunningTaskMode() \
    (kTickGetRunningTask()->ctx.ia32eCtx.vtx.syntheticEvent.delivery.fields.mode)

#define ia32eEmulatorQueueUd() \
    ia32eEmulatorQueueEventSynthetic(false, IA32E_INVALID_OPCODE, IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION, false, 0)

#define ia32eEmulatorQueueGp0()                                                         \
    ia32eEmulatorQueueEventSynthetic(false, IA32E_GENERAL_PROTECTION_FAULT,             \
                                     IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION, true, 0)

#define ia32eEmulatorQueueAdvance() \
    ia32eEmulatorQueueEventSynthetic(true, 0, IA32E_INTERRUPT_TYPE_OTHER_EVENT, false, 0)

#define ia32eEmulatorVmxPreemptionTimerIsEnabled()                                  \
    (testBitLe(ia32eVmread(IA32E_VTX_VMCS_CTRL_PINBASED_CONTROLS),                  \
                           IA32E_VTX_VMCS_PINBASED_CTLS_VMX_PREEMPTION_TIMER_BIT))

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

/* Emulation helpers */

static
inline 
uint64_t ia32eEmulatorModeApplyIpMask(uint64_t val)
{
    ia32eEmulatorMode_t mode = IA32E_EMULATOR_INVALID;

    mode = ia32eEmulatorRunningTaskMode();

    switch (mode) {

        case IA32E_EMULATOR_16:
        case IA32E_EMULATOR_V8086:
            val &= 0xffff;
            break;
        
        case IA32E_EMULATOR_32:
            val &= 0xffffffff;
            break;

        case IA32E_EMULATOR_64:
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;        
    }

    return val;
}

static 
inline 
uint64_t ia32eEmulatorReadGpr(ia32eVtxVmcsGpr_t gpr, ia32eVmexitRegs_t *regs)
{
    uint64_t val = 0;

    switch (gpr) {

        case IA32E_VTX_VMCS_GPR_RAX:
            val = regs->regs.rax;
            break;

        case IA32E_VTX_VMCS_GPR_RCX:
            val = regs->regs.rcx;
            break;

        case IA32E_VTX_VMCS_GPR_RDX:
            val = regs->regs.rdx;
            break;

        case IA32E_VTX_VMCS_GPR_RBX:
            val = regs->regs.rbx;
            break;

        case IA32E_VTX_VMCS_GPR_RSP:
            val = ia32eVmread(IA32E_VTX_VMCS_GUEST_RSP);
            break;

        case IA32E_VTX_VMCS_GPR_RBP:
            val = regs->regs.rbp;
            break;

        case IA32E_VTX_VMCS_GPR_RSI:
            val = regs->regs.rsi;
            break;

        case IA32E_VTX_VMCS_GPR_RDI:
            val = regs->regs.rdi;
            break;

        case IA32E_VTX_VMCS_GPR_R8:
            val = regs->regs.r8;
            break;

        case IA32E_VTX_VMCS_GPR_R9:
            val = regs->regs.r9;
            break;

        case IA32E_VTX_VMCS_GPR_R10:
            val = regs->regs.r10;
            break;

        case IA32E_VTX_VMCS_GPR_R11:
            val = regs->regs.r11;
            break;

        case IA32E_VTX_VMCS_GPR_R12:
            val = regs->regs.r12;
            break;

        case IA32E_VTX_VMCS_GPR_R13:
            val = regs->regs.r13;
            break;

        case IA32E_VTX_VMCS_GPR_R14:
            val = regs->regs.r14;
            break;

        case IA32E_VTX_VMCS_GPR_R15:
            val = regs->regs.r15;
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }

    return val;
}

static 
inline 
void ia32eEmulatorWriteGpr(ia32eVtxVmcsGpr_t gpr, ia32eVmexitRegs_t *regs, uint64_t val)
{
    switch (gpr) {

        case IA32E_VTX_VMCS_GPR_RAX:
            regs->regs.rax = val;
            break;

        case IA32E_VTX_VMCS_GPR_RCX:
            regs->regs.rcx = val;
            break;

        case IA32E_VTX_VMCS_GPR_RDX:
            regs->regs.rdx = val;
            break;

        case IA32E_VTX_VMCS_GPR_RBX:
            regs->regs.rbx = val;
            break;

        case IA32E_VTX_VMCS_GPR_RSP:
            __ia32eVmwrite(IA32E_VTX_VMCS_GPR_RSP, val);
            break;

        case IA32E_VTX_VMCS_GPR_RBP:
            regs->regs.rbp = val;
            break;

        case IA32E_VTX_VMCS_GPR_RSI:
            regs->regs.rsi = val;
            break;

        case IA32E_VTX_VMCS_GPR_RDI:
            regs->regs.rdi = val;
            break;

        case IA32E_VTX_VMCS_GPR_R8:
            regs->regs.r8 = val;
            break;

        case IA32E_VTX_VMCS_GPR_R9:
            regs->regs.r9 = val;
            break;

        case IA32E_VTX_VMCS_GPR_R10:
            regs->regs.r10 = val;
            break;

        case IA32E_VTX_VMCS_GPR_R11:
            regs->regs.r11 = val;
            break;

        case IA32E_VTX_VMCS_GPR_R12:
            regs->regs.r12 = val;
            break;

        case IA32E_VTX_VMCS_GPR_R13:
            regs->regs.r13 = val;
            break;

        case IA32E_VTX_VMCS_GPR_R14:
            regs->regs.r14 = val;
            break;

        case IA32E_VTX_VMCS_GPR_R15:
            regs->regs.r15 = val;
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }
}

static 
inline
ia32eEmulatorMode_t ia32eEmulatorMode(void)
{
    uint64_t guestCr0 = 0;
    uint64_t guestEfer = 0;
    uint32_t guestAr = 0;
    uint64_t guestFlags = 0;

    guestCr0 = ia32eVmread(IA32E_VTX_VMCS_GUEST_CR0);
    if ((guestCr0 & IA32E_CR0_PE_MASK) == 0)
        return IA32E_EMULATOR_16;

    guestEfer = ia32eVmread(IA32E_VTX_VMCS_GUEST_IA32E_EFER);
    if ((guestEfer & IA32E_EFER_LONGMODE_ACTIVE_MASK) != 0) {
        
        guestAr = ia32eVmread(IA32E_VTX_VMCS_GUEST_CS_ACCESS_RIGHTS);
        return (guestAr & IA32E_ACCESS_RIGHTS_LONGMODE_MASK) != 0 ? IA32E_EMULATOR_64 : 
               (guestAr & IA32E_ACCESS_RIGHTS_DB_MASK) != 0 ? IA32E_EMULATOR_32 : IA32E_EMULATOR_16;
    }

    guestFlags = ia32eVmread(IA32E_VTX_VMCS_GUEST_RFLAGS);
    if ((guestFlags & IA32E_FLAGS_VM_MASK) != 0)
        return IA32E_EMULATOR_V8086;

    guestAr = ia32eVmread(IA32E_VTX_VMCS_GUEST_CS_ACCESS_RIGHTS);
    return (guestAr & IA32E_ACCESS_RIGHTS_DB_MASK) != 0 ? IA32E_EMULATOR_32 : IA32E_EMULATOR_16;
}

static
inline
bool ia32eEmulatorCpl0(void)
{
    uint64_t guestCr0 = 0;
    uint64_t guestEfer = 0;
    uint64_t guestFlags = 0;
    uint32_t guestAr = 0;

    guestCr0 = ia32eVmread(IA32E_VTX_VMCS_GUEST_CR0);
    if ((guestCr0 & IA32E_CR0_PE_MASK) == 0)
        return true;

    guestEfer = ia32eVmread(IA32E_VTX_VMCS_GUEST_IA32E_EFER);
    if ((guestEfer & IA32E_EFER_LONGMODE_ACTIVE_MASK) == 0) {

        guestFlags = ia32eVmread(IA32E_VTX_VMCS_GUEST_RFLAGS);
        if ((guestFlags & IA32E_FLAGS_VM_MASK) != 0)
            return false;
    }

    guestAr = ia32eVmread(IA32E_VTX_VMCS_GUEST_CS_ACCESS_RIGHTS);
    return (guestAr & IA32E_ACCESS_RIGHTS_DPL_MASK) == 0;
}

static
inline
void ia32eEmulatorInjectEvent(uint8_t vector, ia32eInterruptType_t type, bool deliverErrcode, uint64_t errcode, 
                              bool deliverLength, uint64_t length)
{
    uint32_t info = 0;
    
    info = vector | (type << 8) | (deliverErrcode ? (1 << 11) : 0) | (1 << 31);

#if CONFIG_KDYNAMIC_ASSERT
    
    K_DYNAMIC_ASSERT(__ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMENTRY_INTERRUPT_INFO, info));

    if (deliverErrcode)
        K_DYNAMIC_ASSERT(__ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMENTRY_INTERRUPT_ERROR_CODE, errcode));

    if (deliverLength)
        K_DYNAMIC_ASSERT(__ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMENTRY_INSTRUCTION_LENGTH, length));

#else 

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMENTRY_INTERRUPT_INFO, info);

    if (deliverErrcode)
        __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMENTRY_INTERRUPT_ERROR_CODE, errcode);

    if (deliverLength)
        __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_VMENTRY_INSTRUCTION_LENGTH, length);

#endif
}

static
inline
void ia32eEmulatorAdvance(ia32eVmexitRegs_t *regs)
{
    uintptr_t guestIp = 0;
    uintptr_t length = 0;
    uint64_t guestFlags = 0;
    uint32_t interruptibilityState = 0;

    guestIp = ia32eVmread(IA32E_VTX_VMCS_GUEST_RIP);
    length = ia32eVmread(IA32E_VTX_VMCS_RO_VMEXIT_INSTRUCTION_LENGTH);
    guestFlags = ia32eVmread(IA32E_VTX_VMCS_GUEST_RFLAGS);
    interruptibilityState = ia32eVmread(IA32E_VTX_VMCS_GUEST_INTERRUPTIBILITY_STATE);

    guestIp += length;

    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_RIP, ia32eEmulatorModeApplyIpMask(guestIp));

    if ((interruptibilityState & IA32E_VTX_VMCS_GUEST_INTERRUPTIBILITY_STATE_STI_MASK) != 0 ||
        (interruptibilityState & IA32E_VTX_VMCS_GUEST_INTERRUPTIBILITY_STATE_MOV_SS_MASK) != 0) {

        interruptibilityState &= ~IA32E_VTX_VMCS_GUEST_INTERRUPTIBILITY_STATE_STI_MASK;
        interruptibilityState &= ~IA32E_VTX_VMCS_GUEST_INTERRUPTIBILITY_STATE_MOV_SS_MASK;

        __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_INTERRUPTIBILITY_STATE, interruptibilityState);
    }

    if ((guestFlags & IA32E_FLAGS_TF_MASK) != 0) {
        regs->dr6 |= IA32E_DR6_BS_MASK;
        ia32eEmulatorInjectEvent(IA32E_DEBUG_EXCEPTION, IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION, false, 0, false, 0);
    }
}

static
inline
void ia32eEmulatorUnsetNmiBlocking(void)
{
    uint32_t interruptibilityState = 0;

    interruptibilityState = ia32eVmread(IA32E_VTX_VMCS_GUEST_INTERRUPTIBILITY_STATE);
    interruptibilityState &= ~IA32E_VTX_VMCS_GUEST_INTERRUPTIBILITY_STATE_NMI_MASK; 
    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_INTERRUPTIBILITY_STATE, interruptibilityState);
}

static
inline 
void ia32eEmulatorSelfIpi(uint8_t vector)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eIdtDescriptor64_t *ia32eIdt64High = NULL;
    uint8_t ist = 0;

    uint64_t rsp = 0; 

    K_DYNAMIC_ASSERT((cpuReadStatus() & IA32E_FLAGS_IF_MASK) == 0);

    cpu = ia32eThisCpuData();
    ia32eIdt64High = cpu->global->cpuDataStructures.idt;
    ist = ia32eIdt64High[vector].ist;

    switch (ist) {

        case 1:
            rsp = (uint64_t)&cpu->intStack.stack[sizeof(cpu->intStack.stack)];
            break;

        case 2:
            rsp = (uint64_t)&cpu->nmiStack.stack[sizeof(cpu->nmiStack.stack)];
            break;

        case 3:
            rsp = (uint64_t)&cpu->doubleFaultStack.stack[sizeof(cpu->doubleFaultStack.stack)];
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }

    __ia32eFakeIsr(rsp, vector, 0);
}

static
inline 
bool ia32eEmulatorValidateCr0(uint64_t cr0)
{
    /** cr0 invariants
     *
     *  mode <> IA32E_EMULATOR_V8086
     * ¬cr0.upper32
     *  ¬cr0.inval
     *  cr0.pg -> cr0.pe
     *  oldCr0.pg -> cr0.pe
     *  cr0.nw -> cr0.cd
     *  mode = IA32E_EMULATOR_64 -> (cr0.pg /\ cr0.pe)
     *  cr4.pcide -> cr0.pg
     */

    ia32eEmulatorMode_t mode = IA32E_EMULATOR_INVALID;
    bool pcide = false;
    bool oldPg = false;

    bool pg = false;
    bool pe = false;
    bool nw = false;
    bool cd = false;

    mode = ia32eEmulatorRunningTaskMode();
    pcide = (ia32eVmread(IA32E_VTX_VMCS_GUEST_CR4) & IA32E_CR4_PCIDE_MASK) != 0;
    oldPg = (ia32eVmread(IA32E_VTX_VMCS_GUEST_CR0) & IA32E_CR0_PG_MASK) != 0;

    pg = (cr0 & IA32E_CR0_PG_MASK) != 0;
    pe = (cr0 & IA32E_CR0_PE_MASK) != 0;
    nw = (cr0 & IA32E_CR0_NW_MASK) != 0;
    cd = (cr0 & IA32E_CR0_CD_MASK) != 0;

    return (mode != IA32E_EMULATOR_V8086 &&
            (cr0 >> 32) == 0 && 
            (cr0 & IA32E_CR0_INVAL_MASK) == 0 &&
            (!pg || pe) &&
            (!oldPg || pe) &&
            (!nw || cd) &&
            (mode != IA32E_EMULATOR_64 || (pg && pe)) &&
            (!pcide || pg));
}

static
inline 
uint8_t ia32eEmulatorVcpuId(void)
{
    kSchedTask_t *task = NULL;
    kSchedThread_t *thread = NULL;
    kSchedLsr_t *lsr = NULL;

    uint8_t vcpuId = 0;

    task = kTickGetRunningTask();

    switch (task->taggedInfo.type) {

        case K_TASK_THREAD:
            thread = &task->taggedInfo.info.thread;
            vcpuId = thread->archInfo.ia32eInfo.vtxInfo.vcpuId;
            break;

        case K_TASK_LSR:
            lsr = &task->taggedInfo.info.lsr;
            vcpuId = lsr->archInfo.ia32eInfo.vtxInfo.vcpuId;
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }

    return vcpuId;
}

static 
inline 
void ia32eEmulatorQueueEventSynthetic(bool advance, uint8_t vector, ia32eInterruptType_t type, 
                                      bool deliverErrcode, uint64_t errcode)
{
    kSchedTask_t *task = NULL;

    task = kTickGetRunningTask();

    task->ctx.ia32eCtx.vtx.syntheticEvent.delivery.fields.valid = 1;

    if (advance) {
        task->ctx.ia32eCtx.vtx.syntheticEvent.delivery.fields.advance = 1;
        return;
    }

    task->ctx.ia32eCtx.vtx.syntheticEvent.delivery.fields.vector = vector;
    task->ctx.ia32eCtx.vtx.syntheticEvent.delivery.fields.type = type;
    task->ctx.ia32eCtx.vtx.syntheticEvent.delivery.fields.deliverErrcode = deliverErrcode;
    task->ctx.ia32eCtx.vtx.syntheticEvent.errcode = errcode;
}

static 
inline
void ia32eEmulatorCatchLostEvent(void)
{
    uint32_t info = 0;
    
    uint8_t vector = 0;
    ia32eInterruptType_t type = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION;
    
    bool deliverErrcode = false;
    uint64_t errcode = 0;

    kSchedTask_t *task = NULL;

    info = ia32eVmread(IA32E_VTX_VMCS_RO_IDT_VECTORING_INFO_FIELD);

    if ((info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_VALID_MASK) == 0)
        return;

    vector = info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_VECTOR_MASK;    

    type = ((info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_EVENT_TYPE_MASK) >> 
             IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_EVENT_TYPE_SHIFT);

    deliverErrcode = (info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_ERRCODE_MASK) != 0;
    if (deliverErrcode)
        errcode = ia32eVmread(IA32E_VTX_VMCS_RO_IDT_VECTORING_ERROR_CODE);

    task = kTickGetRunningTask();    

    task->ctx.ia32eCtx.vtx.lostEvent.delivery.fields.valid = 1;
    task->ctx.ia32eCtx.vtx.lostEvent.delivery.fields.vector = vector;
    task->ctx.ia32eCtx.vtx.lostEvent.delivery.fields.type = type;
    task->ctx.ia32eCtx.vtx.lostEvent.delivery.fields.deliverErrcode = deliverErrcode;
    task->ctx.ia32eCtx.vtx.lostEvent.errcode = errcode;
}

static
inline
void ia32eEmulatoLoadHostDrx(void)
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

static
inline 
void ia32eEmulatorHandleVcpuFailure(void)
{
    kSchedTask_t *task = NULL;
    kDomain_t *domain = NULL;

    cpuEnableInterrupts();

    task = kTickGetRunningTask();
    domain = task->domain.curDomain;

    K_DYNAMIC_ASSERT(domain);

    atomic_store(&domain->archInfo.ia32eInfo.tripleFault, 1);

    kSyscallHandler(WORKHORSE_SYS_SCHED_CTRL, WORKHORSE_SCHED_CTRL_FAILURE, 0);
}

static
inline 
void ia32eEmulatorX2apicSetTpr(uint8_t val)
{
    kSchedTask_t *task = NULL;

    K_DYNAMIC_ASSERT(val <= 15);

    task = kTickGetRunningTask();
    task->ctx.ia32eCtx.vtx.x2apic.local.fields.tpr = val;
}

static
inline 
uint8_t ia32eEmulatorX2apicGetTpr(void)
{
    kSchedTask_t *task = NULL;
    uint8_t val = 0;

    task = kTickGetRunningTask();
    val = task->ctx.ia32eCtx.vtx.x2apic.local.fields.tpr;

    K_DYNAMIC_ASSERT(val <= 15);

    return val;
}

static
inline
uint8_t ia32eEmulatorX2apicGetIsrv(void)
{
    kSchedTask_t *task = NULL;

    int32_t i = 0;
    int idx = -1;

    task = kTickGetRunningTask();

    for (i = (ARRAY_LEN(task->ctx.ia32eCtx.vtx.x2apic.isr) - 1); i >= 0; i--) {
        
        idx = fls32(task->ctx.ia32eCtx.vtx.x2apic.isr[i]);
        if (idx >= 0)
            return (i * 32) + idx;
    } 

    return 0;
}

static
inline
void ia32eEmulatorX2apicUnsetIsrv(void)
{
    kSchedTask_t *task = NULL;

    int32_t i = 0;
    int idx = -1;

    task = kTickGetRunningTask();

    for (i = (ARRAY_LEN(task->ctx.ia32eCtx.vtx.x2apic.isr) - 1); i >= 0; i--) {
        
        idx = fls32(task->ctx.ia32eCtx.vtx.x2apic.isr[i]);
        if (idx >= 0) {
            task->ctx.ia32eCtx.vtx.x2apic.isr[i] &= ~(1 << idx);
            break;
        }
    } 
}

static
inline
uint32_t ia32eEmulatorX2apicCompressCounter(uint32_t count, uint32_t dcr)
{
    switch (dcr) {
        
        case IA32E_XAPIC_DIV_2:
            count /= 2;
            break;

        case IA32E_XAPIC_DIV_4:
            count /= 4;
            break;

        case IA32E_XAPIC_DIV_8:
            count /= 8;
            break;

        case IA32E_XAPIC_DIV_16:
            count /= 16;
            break;

        case IA32E_XAPIC_DIV_32:
            count /= 32;
            break;

        case IA32E_XAPIC_DIV_64:
            count /= 64;
            break;

        case IA32E_XAPIC_DIV_128:
            count /= 128;
            break;

        case IA32E_XAPIC_DIV_1:
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }
    
    return count;
}

static
inline
uint32_t ia32eEmulatorX2apicInflateCounter(uint32_t count, uint32_t dcr)
{
    switch (dcr) {
        
        case IA32E_XAPIC_DIV_2:
            count *= 2;
            break;

        case IA32E_XAPIC_DIV_4:
            count *= 4;
            break;

        case IA32E_XAPIC_DIV_8:
            count *= 8;
            break;

        case IA32E_XAPIC_DIV_16:
            count *= 16;
            break;

        case IA32E_XAPIC_DIV_32:
            count *= 32;
            break;

        case IA32E_XAPIC_DIV_64:
            count *= 64;
            break;

        case IA32E_XAPIC_DIV_128:
            count *= 128;
            break;

        case IA32E_XAPIC_DIV_1:
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }

    return count;    
}

static
inline
void ia32eEmulatorX2apicResetCounter(void)
{
    kSchedTask_t *task = NULL;
    uint32_t dcr = 0;
    uint32_t initCount = 0;

    uint64_t pin = 0;    
    uint64_t exit = 0;

    task = kTickGetRunningTask();
    dcr = task->ctx.ia32eCtx.vtx.x2apic.dcr;
    initCount = task->ctx.ia32eCtx.vtx.x2apic.initCount;

    pin = ia32eVmread(IA32E_VTX_VMCS_CTRL_PINBASED_CONTROLS);
    exit = ia32eVmread(IA32E_VTX_VMCS_CTRL_PRIMARY_VMEXIT_CONTROLS);

    if (initCount == 0) {

        if (testBitLe(pin, IA32E_VTX_VMCS_PINBASED_CTLS_VMX_PREEMPTION_TIMER_BIT)) {

            __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE, 0);

            pin &= ~(1 << IA32E_VTX_VMCS_PINBASED_CTLS_VMX_PREEMPTION_TIMER_BIT);
            exit &= ~(1 << IA32E_VTX_VMCS_EXIT_CTLS_SAVE_VMX_PREEMPTION_TIMER_BIT);

            __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_PINBASED_CONTROLS, pin);
            __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_PRIMARY_VMEXIT_CONTROLS, exit);
        }

        return;
    }

    initCount = ia32eEmulatorX2apicCompressCounter(initCount, dcr);

    if (!testBitLe(pin, IA32E_VTX_VMCS_PINBASED_CTLS_VMX_PREEMPTION_TIMER_BIT)) {

        pin |= (1 << IA32E_VTX_VMCS_PINBASED_CTLS_VMX_PREEMPTION_TIMER_BIT);
        exit |= (1 << IA32E_VTX_VMCS_EXIT_CTLS_SAVE_VMX_PREEMPTION_TIMER_BIT);

        __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_PINBASED_CONTROLS, pin);
        __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_PRIMARY_VMEXIT_CONTROLS, exit);
    }

    __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE, initCount);
}

static
inline
uint32_t ia32eEmulatorX2apicReadCounter(void)
{
    kSchedTask_t *task = NULL;
    uint32_t dcr = 0;
    
    uint32_t count = 0;

    if (!ia32eEmulatorVmxPreemptionTimerIsEnabled())
        return 0;

    task = kTickGetRunningTask();
    dcr = task->ctx.ia32eCtx.vtx.x2apic.dcr;

    count = ia32eVmread(IA32E_VTX_VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE);
    count = ia32eEmulatorX2apicInflateCounter(count, dcr);

    return count;
}

static
inline
void ia32eEmulatorX2apicSetDcr(uint8_t dcr)
{
    kSchedTask_t *task = NULL;
    uint8_t oldDcr = 0;

    uint32_t count = 0;

    task = kTickGetRunningTask();

    oldDcr = task->ctx.ia32eCtx.vtx.x2apic.dcr;
    task->ctx.ia32eCtx.vtx.x2apic.dcr = dcr;

    if (dcr != oldDcr && ia32eEmulatorVmxPreemptionTimerIsEnabled()) {

        count = ia32eVmread(IA32E_VTX_VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE);
        count = ia32eEmulatorX2apicInflateCounter(count, oldDcr);
        count = ia32eEmulatorX2apicCompressCounter(count, dcr);
        __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE, count);
    }
}

/* General purpose events */

static 
void ia32eEmulatorUd(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
    ia32eEmulatorQueueUd();
}

static 
void ia32eEmulatorNop(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
    cpuRelax();
}

static 
void ia32eEmulatorNopAdvance(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
    ia32eEmulatorQueueAdvance();
}

ATTR_NORETURN
static
void ia32eEmulatorVcpuFailure(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
    ia32eEmulatorHandleVcpuFailure();
    UNREACHABLE();
}

static 
void ia32eEmulatorAccessDenied(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
#if CONFIG_IA32E_VTX_ACCESS_DENIED_GP0
    ia32eEmulatorQueueGp0();
#else 
    ia32eEmulatorHandleVcpuFailure();
#endif 
}

/* Specific events */

static 
void ia32eEmulatorException(ia32eVmexitRegs_t *regs)
{
    uint32_t info = 0;

    uint8_t vector = 0;
    ia32eInterruptType_t type = IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION;
    
    bool deliverErrcode = false;
    uint64_t errcode = 0;

    uint64_t qual = 0;

    info = ia32eVmread(IA32E_VTX_VMCS_RO_VMEXIT_INTERRUPT_INFO);

    K_DYNAMIC_ASSERT((info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_VALID_MASK) != 0);

    vector = info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_VECTOR_MASK;    

    type = ((info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_EVENT_TYPE_MASK) >> 
             IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_EVENT_TYPE_SHIFT);

    if ((info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_NMI_UNBLOCKING_MASK) != 0)
        ia32eEmulatorUnsetNmiBlocking();
            
    if (type == IA32E_INTERRUPT_TYPE_NMI) {
        K_DYNAMIC_ASSERT(vector == IA32E_NMI);

        ia32eEmulatorSelfIpi(vector);
        cpuEnableInterrupts();
        return;
    }

    cpuEnableInterrupts();

    K_DYNAMIC_ASSERT(vector == IA32E_DEBUG_EXCEPTION || vector == IA32E_ALIGNMENT_CHECK);

    deliverErrcode = (info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_ERRCODE_MASK) != 0;
    if (deliverErrcode)
        errcode = ia32eVmread(IA32E_VTX_VMCS_RO_VMEXIT_INTERRUPT_ERROR_CODE);

    if (vector == IA32E_DEBUG_EXCEPTION) {
        qual = ia32eVmread(IA32E_VTX_VMCS_RO_EXIT_QUALIFICATION);

        regs->dr6 |= (qual & IA32E_EMULATOR_DB_DR6_TARGET_MASK);
        regs->dr6 &= ~(qual & IA32E_DR6_BLD_MASK);

        if ((qual & IA32E_DR6_RTM_MASK) != 0)
            regs->dr6 &= ~IA32E_DR6_RTM_MASK;
        else
            regs->dr6 |= IA32E_DR6_RTM_MASK;
    }

    ia32eEmulatorQueueEventSynthetic(false, vector, type, deliverErrcode, errcode);
}

static 
void ia32eEmulatorExtIntr(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
    uint32_t info = 0;
    uint8_t vector = 0;

    info = ia32eVmread(IA32E_VTX_VMCS_RO_VMEXIT_INTERRUPT_INFO);

    K_DYNAMIC_ASSERT((info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_VALID_MASK) != 0);
    
    vector = info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_VECTOR_MASK;

    if ((info & IA32E_VTX_VMCS_VECTORED_EVENTS_INFO_NMI_UNBLOCKING_MASK) != 0)
        ia32eEmulatorUnsetNmiBlocking();

    ia32eEmulatorSelfIpi(vector);
    cpuEnableInterrupts();
}

static 
void ia32eEmulatorCpuid(ia32eVmexitRegs_t *regs)
{
    uint32_t eax = 0;
    uint32_t ecx = 0;

    ia32ePerCpu_t *cpu = NULL;
    uint32_t vcpuId = 0;

    uint32_t cpuidRegs[4] = {0};

    eax = regs->regs.rax & 0xffffffff;
    ecx = regs->regs.rcx & 0xffffffff;

    cpu = ia32eThisCpuData();
    vcpuId = ia32eEmulatorVcpuId();

    regs->regs.rax = 0;
    regs->regs.rbx = 0;
    regs->regs.rcx = 0;
    regs->regs.rdx = 0;

    ia32eEmulatorQueueAdvance();

    if (eax >= IA32E_EMULATOR_CPUIDV_START && eax <= IA32E_EMULATOR_CPUIDV_EMULATION) {
        
        switch (eax) {

            case IA32E_EMULATOR_CPUIDV_START:
                regs->regs.rax = IA32E_EMULATOR_CPUIDV_EMULATION;
                regs->regs.rbx = IA32E_EMULATOR_CPUIDV_EBX;
                regs->regs.rcx = IA32E_EMULATOR_CPUIDV_ECX;
                regs->regs.rdx = IA32E_EMULATOR_CPUIDV_EDX;
                break;

            case IA32E_EMULATOR_CPUIDV_EMULATION:

#if CONFIG_IA32E_VTX_ACCESS_DENIED_GP0
                regs-regs.rdx |= IA32E_EMULATOR_CPUIDV_EMULATION_D_ACCESS_DENIED_GP0_MASK;
#endif            
                break;

            default:
                break;
        }

        return;
    }

    if (eax >= IA32E_CPUID_ESIG0 && eax <= min(IA32E_CPUID_ESIG8, cpu->esigMax)) {
        
        switch (eax) {

            case IA32E_CPUID_ESIG0:
                regs->regs.rax = min(IA32E_CPUID_ESIG8, cpu->esigMax);
                break;

            case IA32E_CPUID_ESIG1:

                regs->regs.rdx = IA32E_CPUID_ESIG1_D_SYSCALL_MASK | IA32E_CPUID_ESIG1_D_INTEL64_MASK;
                
                if (cpu->cpuFlags.fields.lahf64 != 0)
                    regs->regs.rcx |= IA32E_CPUID_ESIG1_C_LAHF64_MASK;

                if (cpu->cpuFlags.fields.lzcnt != 0)
                    regs->regs.rcx |= IA32E_CPUID_ESIG1_C_LZCNT_MASK;

                if (cpu->cpuFlags.fields.prefetchw != 0)
                    regs->regs.rcx |= IA32E_CPUID_ESIG1_C_PREFETCHW_MASK;

                if (cpu->cpuFlags.fields.nx != 0)
                    regs->regs.rdx |= IA32E_CPUID_ESIG1_D_NX_MASK;

                if (cpu->cpuFlags.fields.pg1Gb != 0)
                    regs->regs.rdx |= IA32E_CPUID_ESIG1_D_PG_1GB_MASK;

                break;

            case IA32E_CPUID_ESIG2:
            case IA32E_CPUID_ESIG3:
            case IA32E_CPUID_ESIG4:
                ia32eCpuid(eax, ecx, &cpuidRegs[0], &cpuidRegs[1], &cpuidRegs[2], &cpuidRegs[3]);
                regs->regs.rax = cpuidRegs[0];
                regs->regs.rbx = cpuidRegs[1];
                regs->regs.rcx = cpuidRegs[2];
                regs->regs.rdx = cpuidRegs[3];
                break;

            case IA32E_CPUID_ESIG6:
                regs->regs.rcx = cpu->cpuFlags.fields.l2LineSize;
                break;

            case IA32E_CPUID_ESIG8:
                regs->regs.rax = 48 | (48 << 8) | (48 << 16);
                regs->regs.rbx = (cpu->cpuFlags.fields.wbnoinvd << 9);
                break;

            default:
                break;
        }

        return;
    }

    switch (eax) {

        case 0:
            regs->regs.rax = cpu->cpuFlags.fields.avx10 != 0 ? 36 : 7;
            regs->regs.rbx = IA32E_CPUID0_GENUINE_INTEL_EBX;
            regs->regs.rcx = IA32E_CPUID0_GENUINE_INTEL_ECX;
            regs->regs.rdx = IA32E_CPUID0_GENUINE_INTEL_EDX;
            break;

        case 1:
            ia32eCpuid(1, 0, &cpuidRegs[0], &cpuidRegs[1], &cpuidRegs[2], &cpuidRegs[3]);

            regs->regs.rax = cpuidRegs[0];
            regs->regs.rbx = (cpuidRegs[1] & 0xffff) | (vcpuId << 24);

            regs->regs.rcx |= (cpuidRegs[2] & IA32E_EMULATOR_CPUID1_C_TARGET_MASK);
            regs->regs.rcx |= IA32E_CPUID1_C_X2APIC_MASK;
            regs->regs.rcx |= IA32E_CPUID1_C_HYPERVISOR_PRESENT_MASK;

            regs->regs.rdx |= (cpuidRegs[3] & IA32E_EMULATOR_CPUID1_D_TARGET_MASK);
            regs->regs.rdx |= IA32E_CPUID1_D_APIC_MASK;

#if CONFIG_IA32E_VTX_TSD
            regs->regs.rdx &= ~IA32E_CPUID1_D_TSC_MASK;
#endif
            break;

        case 2:
            regs->regs.rax = (1 << 31);
            regs->regs.rbx = (1 << 31);
            regs->regs.rcx = (1 << 31);
            regs->regs.rdx = (1 << 31);
            break;

        case 3:
        case 4:
        case 5:
        case 6:
            break;

        default:

            if (eax > 7 && cpu->cpuFlags.fields.avx10 != 0) {
            
                if (eax < 36 || ecx > 1)
                    break;

                ia32eCpuid(36, ecx, &cpuidRegs[0], &cpuidRegs[1], &cpuidRegs[2], &cpuidRegs[3]);
                regs->regs.rax = cpuidRegs[0];
                regs->regs.rbx = cpuidRegs[1];
                regs->regs.rcx = cpuidRegs[2];
                regs->regs.rdx = cpuidRegs[3];
                break;
            }
            
            switch (ecx) {

                case 0:

                    /** QUIRKS:
                     *
                     *  Guest may still be able to execute enqcmd/s but it should #GP rather than #UD 
                     */

                    ia32eCpuid(7, 0, &cpuidRegs[0], &cpuidRegs[1], &cpuidRegs[2], &cpuidRegs[3]);
                    regs->regs.rax = 1;
                    regs->regs.rbx |= (cpuidRegs[1] & IA32E_EMULATOR_CPUID7_0_B_TARGET_MASK);
                    regs->regs.rcx |= (cpuidRegs[2] & IA32E_EMULATOR_CPUID7_0_C_TARGET_MASK);
                    regs->regs.rdx |= (cpuidRegs[3] & IA32E_EMULATOR_CPUID7_0_D_TARGET_MASK);

                    regs->regs.rdx |= IA32E_CPUID7_0_D_ARCH_CAP_MASK;
                    break;

                case 1:

                    regs->regs.rax |= IA32E_CPUID7_1_A_BIOS_DONE_MASK;
                    if (cpu->extFeaturesSubleafMax < 1)
                        break;

                    ia32eCpuid(7, 1, &cpuidRegs[0], &cpuidRegs[1], &cpuidRegs[2], &cpuidRegs[3]);
                    regs->regs.rax |= (cpuidRegs[0] & IA32E_EMULATOR_CPUID7_1_A_TARGET_MASK);
                    regs->regs.rdx |= (cpuidRegs[3] & IA32E_EMULATOR_CPUID7_1_D_TARGET_MASK);
                    break;

                default:
                    break;
            }

            break;
    }
}

static 
void ia32eEmulatorVmcall(ia32eVmexitRegs_t *regs)
{
    ia32eEmulatorMode_t mode = IA32E_EMULATOR_INVALID;
    
    if (!ia32eEmulatorCpl0()) {
        ia32eEmulatorQueueGp0();
        return;
    }

    ia32eEmulatorQueueAdvance();

    mode = ia32eEmulatorRunningTaskMode();
    if (mode == IA32E_EMULATOR_64) {
        ia32eSyscallHandler(&regs->regs);
        return;
    }

    /* Guest cant vmcall in v8086 */

    K_DYNAMIC_ASSERT(mode == IA32E_EMULATOR_16 || mode == IA32E_EMULATOR_32);

    regs->regs.rax &= 0xffffffff;
    regs->regs.rdi &= 0xffffffff;
    regs->regs.rsi &= 0xffffffff;

    ia32eSyscallHandler(&regs->regs);

    regs->regs.rax &= 0xffffffff;
}

static 
void ia32eEmulatorCrAccess(ia32eVmexitRegs_t *regs)
{
    uint32_t qual = 0;

    uint32_t cr = 0;
    ia32eVtxVmcsCrAccessType_t type = IA32E_VTX_VMCS_MOV_TO_CR;
    ia32eVtxVmcsGpr_t gpr = IA32E_VTX_VMCS_GPR_RAX;

    uint64_t val = 0;
    bool valid = true;

    qual = ia32eVmread(IA32E_VTX_VMCS_RO_EXIT_QUALIFICATION);

    cr = (qual & IA32E_VTX_VMCS_CR_ACCESS_QUAL_CR_MASK);
    
    type = ((qual & IA32E_VTX_VMCS_CR_ACCESS_QUAL_ACCESS_TYPE_MASK) >>
            IA32E_VTX_VMCS_CR_ACCESS_QUAL_ACCESS_TYPE_SHIFT);

    gpr = ((qual & IA32E_VTX_VMCS_CR_ACCESS_QUAL_GPR_MASK) >>
            IA32E_VTX_VMCS_CR_ACCESS_QUAL_GPR_SHIFT);

    switch (type) {

        case IA32E_VTX_VMCS_MOV_TO_CR:

            switch (cr) {

                case 0:
                    val = ia32eEmulatorReadGpr(gpr, regs);

                    val &= ~(IA32E_CR0_NW_MASK | IA32E_CR0_CD_MASK);
                    val |= IA32E_CR0_NE_MASK;

                    if (ia32eEmulatorValidateCr0(val))
                        __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_CR0, val);
                    else 
                        valid = false;
                
                    break;

                case 4:
                    valid = false;
                    break;

                case 8:
                    val = ia32eEmulatorReadGpr(gpr, regs);
                    if (val <= 15)
                        ia32eEmulatorX2apicSetTpr(val);
                    else
                        valid = false;

                    break;

                default:
                    K_DYNAMIC_ASSERT(false);
                    break;
            }

            break;

        case IA32E_VTX_VMCS_MOV_FROM_CR:
            K_DYNAMIC_ASSERT(cr == 8);

            ia32eEmulatorWriteGpr(gpr, regs, ia32eEmulatorX2apicGetTpr());
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }

    if (!valid) {
        ia32eEmulatorQueueGp0();
        return;
    }

    ia32eEmulatorQueueAdvance();
}

static 
void ia32eEmulatorRdmsr(ia32eVmexitRegs_t *regs)
{
    kSchedTask_t *task = NULL;
    uint32_t ecx = 0;

    uint64_t vcpuId = 0;

    uint64_t val = 0;
    bool valid = true; 

    task = kTickGetRunningTask();

    ecx = regs->regs.rcx & 0xffffffff;

    STATIC_ASSERT(ARRAY_LEN(task->ctx.ia32eCtx.vtx.x2apic.isr) == 8);
    STATIC_ASSERT(ARRAY_LEN(task->ctx.ia32eCtx.vtx.x2apic.irr) == 8);

    switch (ecx) {

        case IA32E_APIC_BASE:

            val = (task->ctx.ia32eCtx.vtx.x2apic.apicBaseAddr | 
                   IA32E_APIC_BASE_ENABLE_X2APIC_MASK |
                   IA32E_APIC_BASE_GLOBAL_EN_MASK);

            if (task->ctx.ia32eCtx.vtx.x2apic.local.fields.apicBaseBsp != 0)
                val |= IA32E_APIC_BASE_BSP_MASK;

            break;

        case IA32E_X2APIC_ID:
            val = ia32eEmulatorVcpuId();
            break;

        case IA32E_X2APIC_VERSION:
            val = 0x15;
            break;

        case IA32E_X2APIC_TPR:
            val = ia32eEmulatorX2apicGetTpr() << 4;
            break;

        case IA32E_X2APIC_PPR:
            val = max(ia32eEmulatorX2apicGetTpr(), ia32eEmulatorX2apicGetIsrv() / 16) << 4;
            break;

        /*
        case IA32E_X2APIC_EOI:
            break;
        */

        case IA32E_X2APIC_LDR:
            vcpuId = ia32eEmulatorVcpuId();
            val = ((vcpuId / 16) << 16) | (1 << (vcpuId % 16));
            break;

        case IA32E_X2APIC_SIVR:
            val = task->ctx.ia32eCtx.vtx.x2apic.sivr;
            break;

        case IA32E_X2APIC_ISR0:
            val = task->ctx.ia32eCtx.vtx.x2apic.isr[0];
            break;
        case IA32E_X2APIC_ISR1:
            val = task->ctx.ia32eCtx.vtx.x2apic.isr[1];
            break;
        case IA32E_X2APIC_ISR2:
            val = task->ctx.ia32eCtx.vtx.x2apic.isr[2];
            break;
        case IA32E_X2APIC_ISR3:
            val = task->ctx.ia32eCtx.vtx.x2apic.isr[3];
            break;
        case IA32E_X2APIC_ISR4:
            val = task->ctx.ia32eCtx.vtx.x2apic.isr[4];
            break;
        case IA32E_X2APIC_ISR5:
            val = task->ctx.ia32eCtx.vtx.x2apic.isr[5];
            break;
        case IA32E_X2APIC_ISR6:
            val = task->ctx.ia32eCtx.vtx.x2apic.isr[6];
            break;
        case IA32E_X2APIC_ISR7:
            val = task->ctx.ia32eCtx.vtx.x2apic.isr[7];
            break;

        case IA32E_X2APIC_TMR0:
        case IA32E_X2APIC_TMR1:
        case IA32E_X2APIC_TMR2:
        case IA32E_X2APIC_TMR3:
        case IA32E_X2APIC_TMR4:
        case IA32E_X2APIC_TMR5:
        case IA32E_X2APIC_TMR6:
        case IA32E_X2APIC_TMR7:
            break;

        case IA32E_X2APIC_IRR0:
            val = atomic_load(&task->ctx.ia32eCtx.vtx.x2apic.irr[0]);
            break;
        case IA32E_X2APIC_IRR1:
            val = atomic_load(&task->ctx.ia32eCtx.vtx.x2apic.irr[1]);
            break;
        case IA32E_X2APIC_IRR2:
            val = atomic_load(&task->ctx.ia32eCtx.vtx.x2apic.irr[2]);
            break;
        case IA32E_X2APIC_IRR3:
            val = atomic_load(&task->ctx.ia32eCtx.vtx.x2apic.irr[3]);
            break;
        case IA32E_X2APIC_IRR4:
            val = atomic_load(&task->ctx.ia32eCtx.vtx.x2apic.irr[4]);
            break;
        case IA32E_X2APIC_IRR5:
            val = atomic_load(&task->ctx.ia32eCtx.vtx.x2apic.irr[5]);
            break;
        case IA32E_X2APIC_IRR6:
            val = atomic_load(&task->ctx.ia32eCtx.vtx.x2apic.irr[6]);
            break;
        case IA32E_X2APIC_IRR7:
            val = atomic_load(&task->ctx.ia32eCtx.vtx.x2apic.irr[7]);
            break;

        case IA32E_X2APIC_ESR:
            val = task->ctx.ia32eCtx.vtx.x2apic.esr;
            break;

        case IA32E_X2APIC_ICR:
            val = task->ctx.ia32eCtx.vtx.x2apic.icr;
            break;

        case IA32E_X2APIC_LVT_TIMER:
            val = task->ctx.ia32eCtx.vtx.x2apic.lvtTImer;
            break;

        case IA32E_X2APIC_TIMER_INIT_COUNT:
            val = task->ctx.ia32eCtx.vtx.x2apic.initCount;
            break;

        case IA32E_X2APIC_TIMER_CUR_COUNT:
            val = ia32eEmulatorX2apicReadCounter();
            break;

        case IA32E_X2APIC_DIV_CONF:
            val = task->ctx.ia32eCtx.vtx.x2apic.dcr;
            break;
            
        /*
        case IA32E_X2APIC_SELF_IPI:
            break;
        */

        case IA32E_ARCH_CAP:
            val = IA32E_ARCH_CAP_XAPIC_DISABLE_STATUS_MASK;
            break;

        case IA32E_XAPIC_DISABLE_STATUS:
            val = 1;
            break;

        case IA32E_BIOS_DONE:
            val = 0x3;
            break;

        default:
            valid = false;
            break;
    }

    if (!valid) {
        ia32eEmulatorQueueGp0();
        return;
    }

    regs->regs.rax = val & 0xffffffff;
    regs->regs.rdx = val >> 32;

    ia32eEmulatorQueueAdvance();
}

static 
void ia32eEmulatorWrmsr(ia32eVmexitRegs_t *regs)
{
    kSchedTask_t *task = NULL;
    
    uint32_t ecx = 0;
    uint64_t eax = 0;
    uint64_t edx = 0;

    uint64_t val = 0;

    bool valid = true;

    task = kTickGetRunningTask();

    ecx = regs->regs.rcx & 0xffffffff;
    eax = regs->regs.rax & 0xffffffff;
    edx = regs->regs.rdx & 0xffffffff;

    val = (edx << 32) | eax;

    switch (ecx) {

        case IA32E_APIC_BASE:
            
            if ((val & ((0xffffULL << 48) | 0xffULL | (1ULL << 9))) != 0 || 
                ((~val) & (IA32E_APIC_BASE_ENABLE_X2APIC_MASK | IA32E_APIC_BASE_GLOBAL_EN_MASK)) != 0) {

                valid = false;
                break;
            }

            task->ctx.ia32eCtx.vtx.x2apic.local.fields.apicBaseBsp = (val & IA32E_APIC_BASE_BSP_MASK) != 0;
            task->ctx.ia32eCtx.vtx.x2apic.apicBaseAddr = val & ~0xfffULL;
            break;

        /*
        case IA32E_X2APIC_ID:
            break;

        case IA32E_X2APIC_VERSION:
            break;
        */

        case IA32E_X2APIC_TPR:
            
            if ((val & ~0xffULL) == 0)
                ia32eEmulatorX2apicSetTpr(val >> 4);
            else
                valid = false;

            break;

        /*
        case IA32E_X2APIC_PPR:
            break;
        */

        case IA32E_X2APIC_EOI:

            if (val == 0)
                ia32eEmulatorX2apicUnsetIsrv();
            else
                valid = false;

            break;

        /*
        case IA32E_X2APIC_LDR:
            break;
        */

        case IA32E_X2APIC_SIVR:

            if ((val & ~0x1ffULL) == 0)
                task->ctx.ia32eCtx.vtx.x2apic.sivr = val;
            else
                valid = false;

            break;

        /*
        case IA32E_X2APIC_ISR:
        case IA32E_X2APIC_ISR1:
        case IA32E_X2APIC_ISR2:
        case IA32E_X2APIC_ISR3:
        case IA32E_X2APIC_ISR4:
        case IA32E_X2APIC_ISR5:
        case IA32E_X2APIC_ISR6:
        case IA32E_X2APIC_ISR7:

        case IA32E_X2APIC_TMR0:
        case IA32E_X2APIC_TMR1:
        case IA32E_X2APIC_TMR2:
        case IA32E_X2APIC_TMR3:
        case IA32E_X2APIC_TMR4:
        case IA32E_X2APIC_TMR5:
        case IA32E_X2APIC_TMR6:
        case IA32E_X2APIC_TMR7:

        case IA32E_X2APIC_IRR0:
        case IA32E_X2APIC_IRR1:
        case IA32E_X2APIC_IRR2:
        case IA32E_X2APIC_IRR3:
        case IA32E_X2APIC_IRR4:
        case IA32E_X2APIC_IRR5:
        case IA32E_X2APIC_IRR6:
        case IA32E_X2APIC_IRR7:
            break;
        */

        case IA32E_X2APIC_ESR:
            
            if (val != 0) {
                valid = false;
                break;
            }

            task->ctx.ia32eCtx.vtx.x2apic.esr = task->ctx.ia32eCtx.vtx.x2apic.shadowEsr;
            task->ctx.ia32eCtx.vtx.x2apic.shadowEsr = 0;
            break;

        case IA32E_X2APIC_ICR:
            
            break;

        case IA32E_X2APIC_LVT_TIMER:
        
            if ((val & IA32E_EMULATOR_X2APIC_LVT_TIMER_WRITE_RESERVED_MASK) == 0)
                task->ctx.ia32eCtx.vtx.x2apic.lvtTImer = val;
            else
                valid = false;

            break;
      
        case IA32E_X2APIC_TIMER_INIT_COUNT:

            if ((val & ~0xffffffffULL) != 0) {
                valid = false;
                break;
            }

            task->ctx.ia32eCtx.vtx.x2apic.initCount = val;
            ia32eEmulatorX2apicResetCounter();
            break;

        /*
        case IA32E_X2APIC_TIMER_CUR_COUNT:
            break;
        */

        case IA32E_X2APIC_DIV_CONF:

            if ((val & ((~0xfULL) | (1ULL << 2))) == 0)
                ia32eEmulatorX2apicSetDcr(val);
            else 
                valid = false;

            break;

        case IA32E_X2APIC_SELF_IPI:

            if ((val & ~0xffULL) != 0) {
                valid = false;
                break;
            }

            if (val > 15)
                atomic_fetch_or(&task->ctx.ia32eCtx.vtx.x2apic.irr[val / 32], (1 << (val % 32)));
            else
                task->ctx.ia32eCtx.vtx.x2apic.shadowEsr |= IA32E_XAPIC_ESR_SEND_ILLEGAL_MASK;

            break;

        /*
        case IA32E_ARCH_CAP:
        case IA32E_XAPIC_DISABLE_STATUS:
        case IA32E_BIOS_DONE:
            valid = false;
            break;
        */

        default:
            valid = false;
            break;
    }

    if (valid)
        ia32eEmulatorQueueAdvance();
    else
        ia32eEmulatorQueueGp0();
        
}

/* MCE's currently not supported, processor will enter shutdown upon one anyway */

static 
void ia32eEmulatorMceDuringVmentry(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
    cpuRelax();
}

static 
void ia32eEmulatorVmxPreempt(ATTR_UNUSED ia32eVmexitRegs_t *regs)
{
    kSchedTask_t *task = NULL;
    uint32_t lvtTImer = 0;

    uint32_t initCount = 0;
    uint32_t dcr = 0;

    uint8_t vector = 0;

    uint64_t pin = 0;
    uint64_t exit = 0;

    task = kTickGetRunningTask();
    lvtTImer = task->ctx.ia32eCtx.vtx.x2apic.lvtTImer;

    initCount = task->ctx.ia32eCtx.vtx.x2apic.initCount;
    dcr = task->ctx.ia32eCtx.vtx.x2apic.dcr;

    if ((lvtTImer & IA32E_XAPIC_LVT_TIMER_ENABLE_MASK) != 0) {
        vector = lvtTImer & IA32E_XAPIC_LVT_TIMER_VECTOR_MASK;
        ia32eEmulatorQueueEventSynthetic(false, vector, IA32E_INTERRUPT_TYPE_EXTERNAL, false, 0);
    }

    pin = ia32eVmread(IA32E_VTX_VMCS_CTRL_PINBASED_CONTROLS);
    exit = ia32eVmread(IA32E_VTX_VMCS_CTRL_PRIMARY_VMEXIT_CONTROLS);

    K_DYNAMIC_ASSERT(testBitLe(pin, IA32E_VTX_VMCS_PINBASED_CTLS_VMX_PREEMPTION_TIMER_BIT));
    K_DYNAMIC_ASSERT(ia32eVmread(IA32E_VTX_VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE) == 0);

    if ((lvtTImer & IA32E_XAPIC_LVT_TIMER_PERIODIC_MASK) != 0) {

        initCount = ia32eEmulatorX2apicCompressCounter(initCount, dcr);
        __ia32eVmwrite(IA32E_VTX_VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE, lvtTImer);
        return;
    }

    pin &= ~(1 << IA32E_VTX_VMCS_PINBASED_CTLS_VMX_PREEMPTION_TIMER_BIT);
    exit &= ~(1 << IA32E_VTX_VMCS_EXIT_CTLS_SAVE_VMX_PREEMPTION_TIMER_BIT);

    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_PINBASED_CONTROLS, pin);
    __ia32eVmwrite(IA32E_VTX_VMCS_CTRL_PRIMARY_VMEXIT_CONTROLS, exit);
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
void ia32eEmulatorEventManager(void)
{

}

ATTR_NORETURN
void ia32eEmulatorVcpuFailureEntry(void)
{
    ia32eEmulatoLoadHostDrx();

    ia32eEmulatorHandleVcpuFailure();

    UNREACHABLE();
}

void ia32eEmulatorDispatcher(ia32eVmexitRegs_t *regs)
{
    uint32_t exitReason = 0;
    uint32_t basicReason = 0;
    bool failure = 0;

    kSchedTask_t *task = NULL;
    ia32eEmulatorMode_t mode = IA32E_EMULATOR_INVALID;

    ia32eEmulatorFn_t emulatorFn = NULL;

    ia32eEmulatoLoadHostDrx();

    exitReason = ia32eVmread(IA32E_VTX_VMCS_RO_EXIT_REASON);
    basicReason = exitReason & IA32E_VTX_VMCS_EXIT_REASON_MASK;
    failure = (exitReason & IA32E_VTX_VMCS_EXIT_REASON_VMENTRY_FAILURE_MASK) != 0;

    K_DYNAMIC_ASSERT((exitReason & IA32E_VTX_VMCS_EXIT_REASON_ENCLAVE_MASK) == 0);

    if (failure || basicReason >= ARRAY_LEN(ia32eEmulatorDispatchTable)) {
        ia32eEmulatorHandleVcpuFailure();
        UNREACHABLE();
    }

    K_DYNAMIC_ASSERT(ia32eEmulatorDispatchTable[basicReason]);

    if (!ia32eEmulatorDispatchTable[basicReason]) {
        ia32eEmulatorHandleVcpuFailure();
        UNREACHABLE();
    }
    
    if (basicReason != IA32E_VTX_EXIT_REASON_EXCEPTION && basicReason != IA32E_VTX_EXIT_REASON_EXT_INTR)
        cpuEnableInterrupts();

    task = kTickGetRunningTask();
    mode = ia32eEmulatorMode();

    K_DYNAMIC_ASSERT(mode != IA32E_EMULATOR_INVALID);

    task->ctx.ia32eCtx.vtx.syntheticEvent.delivery.fields.mode = mode;

    ia32eEmulatorCatchLostEvent();

    emulatorFn = ia32eEmulatorDispatchTable[basicReason];
    emulatorFn(regs);

    K_DYNAMIC_ASSERT((cpuReadStatus() & IA32E_FLAGS_IF_MASK) != 0);

    ia32eEmulatorEventManager();
}

#endif