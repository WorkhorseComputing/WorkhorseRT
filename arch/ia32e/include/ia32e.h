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

#ifndef _IA32E_H_
#define _IA32E_H_

/* Paging */

#define IA32E_PT_ENTRY_COUNT 512

#define IA32E_PAGE_SHIFT_4KB 12
#define IA32E_PAGE_SHIFT_2MB 21
#define IA32E_PAGE_SHIFT_1GB 30

#define IA32E_PAGE_SIZE_4KB (1 << IA32E_PAGE_SHIFT_4KB)
#define IA32E_PAGE_SIZE_2MB (1 << IA32E_PAGE_SHIFT_2MB)
#define IA32E_PAGE_SIZE_1GB (1 << IA32E_PAGE_SHIFT_1GB)

#define IA32E_PG_ENTRY_PRESENT_MASK (1ULL << 0)
#define IA32E_PG_ENTRY_RW_MASK      (1ULL << 1)
#define IA32E_PG_ENTRY_US_MASK      (1ULL << 2)
#define IA32E_PG_ENTRY_PWT_MASK     (1ULL << 3)
#define IA32E_PG_ENTRY_PCD_MASK     (1ULL << 4)
#define IA32E_PG_ENTRY_PS_MASK      (1ULL << 7)
#define IA32E_PG_ENTRY_PAT_MASK     (1ULL << 7)
#define IA32E_PG_ENTRY_GLOBAL_MASK  (1ULL << 8)
#define IA32E_PG_ENTRY_PS_PAT_MASK  (1ULL << 12)
#define IA32E_PG_ENTRY_XD_MASK      (1ULL << 63)

/* Control registers */

#define IA32E_CR0_PE_MASK       (1 << 0)
#define IA32E_CR0_MP_MASK       (1 << 1)
#define IA32E_CR0_EM_MASK       (1 << 2)
#define IA32E_CR0_NE_MASK       (1 << 5)
#define IA32E_CR0_WP_MASK       (1 << 16)
#define IA32E_CR0_AM_MASK       (1 << 18)
#define IA32E_CR0_NW_MASK       (1 << 29)
#define IA32E_CR0_CD_MASK       (1 << 30)
#define IA32E_CR0_PG_MASK       (1 << 31)
#define IA32E_CR0_INVAL_MASK    0x1ffaffc0

#define IA32E_CR4_VME_MASK          (1 << 0)
#define IA32E_CR4_PVI_MASK          (1 << 1)
#define IA32E_CR4_TSD_MASK          (1 << 2)
#define IA32E_CR4_DE_MASK           (1 << 3)
#define IA32E_CR4_PSE_MASK          (1 << 4)
#define IA32E_CR4_PAE_MASK          (1 << 5)
#define IA32E_CR4_MCE_MASK          (1 << 6)
#define IA32E_CR4_PGE_MASK          (1 << 7)
#define IA32E_CR4_PCE_MASK          (1 << 8)
#define IA32E_CR4_OSFXSR_MASK       (1 << 9)
#define IA32E_CR4_OSXMMEXCPT_MASK   (1 << 10)
#define IA32E_CR4_UMIP_MASK         (1 << 11)
#define IA32E_CR4_LA57_MASK         (1 << 12)
#define IA32E_CR4_VMXE_MASK         (1 << 13)
#define IA32E_CR4_FSGSBASE_MASK     (1 << 16)
#define IA32E_CR4_PCIDE_MASK        (1 << 17)
#define IA32E_CR4_OSXSAVE_MASK      (1 << 18)
#define IA32E_CR4_SMEP_MASK         (1 << 20) 
#define IA32E_CR4_SMAP_MASK         (1 << 21)
#define IA32E_CR4_PKE_MASK          (1 << 22)
#define IA32E_CR4_UINTR_MASK        (1 << 25)

#define IA32E_TPR_MIN   0
#define IA32E_TPR_MAX   15

/* Debug registers */

#define IA32E_DR6_BP0_MASK          (1 << 0ULL)
#define IA32E_DR6_BP1_MASK          (1 << 1ULL)
#define IA32E_DR6_BP2_MASK          (1 << 2ULL)
#define IA32E_DR6_BP3_MASK          (1 << 3ULL)
#define IA32E_DR6_BLD_MASK          (1 << 11ULL)
#define IA32E_DR6_BD_MASK           (1 << 13ULL)
#define IA32E_DR6_BS_MASK           (1 << 14ULL)
#define IA32E_DR6_BT_MASK           (1 << 15ULL)
#define IA32E_DR6_RTM_MASK          (1 << 16ULL)

#define IA32E_DR6_STICKY_MASK       (IA32E_KERNEL_STACK_OF_DR6_MASK | IA32E_INT_STACK_OF_DR6_MASK | \
                                     IA32E_NMI_STACK_OF_DR6_MASK | IA32E_DOUBLE_FAULT_STACK_OF_DR6_MASK | \
                                     IA32E_DR6_BD_MASK | IA32E_DR6_BS_MASK | IA32E_DR6_BT_MASK)

#define IA32E_DR7_L0_MASK           (1 << 0ULL)
#define IA32E_DR7_G0_MASK           (1 << 1ULL)
#define IA32E_DR7_L1_MASK           (1 << 2ULL)
#define IA32E_DR7_G1_MASK           (1 << 3ULL)
#define IA32E_DR7_L2_MASK           (1 << 4ULL)
#define IA32E_DR7_G2_MASK           (1 << 5ULL)
#define IA32E_DR7_L3_MASK           (1 << 6ULL)
#define IA32E_DR7_G3_MASK           (1 << 7ULL)

#define IA32E_DR7_BP0_COND_BIT      16ULL
#define IA32E_DR7_BP0_LEN_BIT       18ULL

#define IA32E_DR7_BP1_COND_BIT      20ULL
#define IA32E_DR7_BP1_LEN_BIT       22ULL

#define IA32E_DR7_BP2_COND_BIT      24ULL
#define IA32E_DR7_BP2_LEN_BIT       26ULL

#define IA32E_DR7_BP3_COND_BIT      28ULL
#define IA32E_DR7_BP3_LEN_BIT       30ULL

#define IA32E_DR7_BP_COND_XO        0ULL
#define IA32E_DR7_BP_COND_WO        1ULL
#define IA32E_DR7_BP_COND_IORW      2ULL
#define IA32E_DR7_BP_COND_RW        3ULL

#define IA32E_DR7_BP_LEN_1B         0ULL
#define IA32E_DR7_BP_LEN_2B         1ULL
#define IA32E_DR7_BP_LEN_8B         2ULL
#define IA32E_DR7_BP_LEN_4B         3ULL

#define IA32E_DR7_BP0_RW_8B_MASK    (IA32E_DR7_G0_MASK |                                \
                                    (IA32E_DR7_BP_COND_RW << IA32E_DR7_BP0_COND_BIT) |  \
                                    (IA32E_DR7_BP_LEN_8B << IA32E_DR7_BP0_LEN_BIT))

#define IA32E_DR7_BP1_RW_8B_MASK    (IA32E_DR7_G1_MASK |                                \
                                    (IA32E_DR7_BP_COND_RW << IA32E_DR7_BP1_COND_BIT) |  \
                                    (IA32E_DR7_BP_LEN_8B << IA32E_DR7_BP1_LEN_BIT))

#define IA32E_DR7_BP2_RW_8B_MASK    (IA32E_DR7_G2_MASK |                                \
                                    (IA32E_DR7_BP_COND_RW << IA32E_DR7_BP2_COND_BIT) |  \
                                    (IA32E_DR7_BP_LEN_8B << IA32E_DR7_BP2_LEN_BIT))

#define IA32E_DR7_BP3_RW_8B_MASK    (IA32E_DR7_G3_MASK |                                \
                                    (IA32E_DR7_BP_COND_RW << IA32E_DR7_BP3_COND_BIT) |  \
                                    (IA32E_DR7_BP_LEN_8B << IA32E_DR7_BP3_LEN_BIT))

/* Cpuid */

#define IA32E_CPUID0_GENUINE_INTEL_EBX  0x756E6547
#define IA32E_CPUID0_GENUINE_INTEL_ECX  0x6C65746E
#define IA32E_CPUID0_GENUINE_INTEL_EDX  0x49656E69

#define IA32E_CPUID1_D_FPU_MASK     (1 << 0)
#define IA32E_CPUID1_D_VME_MASK     (1 << 1)
#define IA32E_CPUID1_D_DE_MASK      (1 << 2)
#define IA32E_CPUID1_D_PSE_MASK     (1 << 3)
#define IA32E_CPUID1_D_TSC_MASK     (1 << 4)
#define IA32E_CPUID1_D_MSR_MASK     (1 << 5)
#define IA32E_CPUID1_D_PAE_MASK     (1 << 6)
#define IA32E_CPUID1_D_CX8_MASK     (1 << 8)
#define IA32E_CPUID1_D_APIC_MASK    (1 << 9)
#define IA32E_CPUID1_D_SEP_MASK     (1 << 11)
#define IA32E_CPUID1_D_MTRR_MASK    (1 << 12)
#define IA32E_CPUID1_D_PGE_MASK     (1 << 13)
#define IA32E_CPUID1_D_CMOV_MASK    (1 << 15)
#define IA32E_CPUID1_D_PAT_MASK     (1 << 16)
#define IA32E_CPUID1_D_PSE36_MASK   (1 << 17)
#define IA32E_CPUID1_D_CLFLUSH_MASK (1 << 19)
#define IA32E_CPUID1_D_MMX_MASK     (1 << 23)
#define IA32E_CPUID1_D_FXSR_MASK    (1 << 24)
#define IA32E_CPUID1_D_SSE_MASK     (1 << 25)
#define IA32E_CPUID1_D_SSE2_MASK    (1 << 26)
#define IA32E_CPUID1_D_SS_MASK      (1 << 27)
#define IA32E_CPUID1_D_HTT_MASK     (1 << 28)
#define IA32E_CPUID1_D_IA64_MASK    (1 << 30)
#define IA32E_CPUID1_D_PBE_MASK     (1 << 31)

#define IA32E_CPUID1_C_SSE3_MASK                (1 << 0)
#define IA32E_CPUID1_C_PCLMULQDQ_MASK           (1 << 1)
#define IA32E_CPUID1_C_DTES64_MASK              (1 << 2)
#define IA32E_CPUID1_C_MONITOR_MWAIT_MASK       (1 << 3)
#define IA32E_CPUID1_C_DS_CPL_MASK              (1 << 4)
#define IA32E_CPUID1_C_VMX_MASK                 (1 << 5)
#define IA32E_CPUID1_C_SMX_MASK                 (1 << 6)
#define IA32E_CPUID1_C_EST_MASK                 (1 << 7)
#define IA32E_CPUID1_C_TM2_MASK                 (1 << 8)
#define IA32E_CPUID1_C_SSSE3_MASK               (1 << 9)
#define IA32E_CPUID1_C_L1_CTX_ID_MASK           (1 << 10)
#define IA32E_CPUID1_C_FMA_MASK                 (1 << 12)
#define IA32E_CPUID1_C_CX16_MASK                (1 << 13)
#define IA32E_CPUID1_C_XTPR_MASK                (1 << 14)
#define IA32E_CPUID1_C_PCID_MASK                (1 << 17)
#define IA32E_CPUID1_C_SSE4_1_MASK              (1 << 19)
#define IA32E_CPUID1_C_SSE4_2_MASK              (1 << 20)
#define IA32E_CPUID1_C_X2APIC_MASK              (1 << 21)
#define IA32E_CPUID1_C_MOVBE_MASK               (1 << 22)
#define IA32E_CPUID1_C_POPCNT_MASK              (1 << 23)
#define IA32E_CPUID1_C_AES_NI_MASK              (1 << 25)
#define IA32E_CPUID1_C_F16C_MASK                (1 << 29)
#define IA32E_CPUID1_C_RDRAND_MASK              (1 << 30)
#define IA32E_CPUID1_C_HYPERVISOR_PRESENT_MASK  (1 << 31)

#define IA32E_CPUID7_0_B_FSGSBASE_MASK              (1 << 0)
#define IA32E_CPUID7_0_B_BMI1_MASK                  (1 << 3)
#define IA32E_CPUID7_0_B_HLE_MASK                   (1 << 4)
#define IA32E_CPUID7_0_B_FDP_EXCEPTION_ONLY_MASK    (1 << 6)
#define IA32E_CPUID7_0_B_SMEP_MASK                  (1 << 7)
#define IA32E_CPUID7_0_B_BMI2_MASK                  (1 << 8)
#define IA32E_CPUID7_0_B_ERMS_MASK                  (1 << 9)
#define IA32E_CPUID7_0_B_INVPCID_MASK               (1 << 10)
#define IA32E_CPUID7_0_B_RTM_MASK                   (1 << 11)
#define IA32E_CPUID7_0_B_FCS_FDS_DEPR_MASK          (1 << 13)
#define IA32E_CPUID7_0_B_RDSEED_MASK                (1 << 18)
#define IA32E_CPUID7_0_B_ADX_MASK                   (1 << 19)
#define IA32E_CPUID7_0_B_SMAP_MASK                  (1 << 20)
#define IA32E_CPUID7_0_B_PCOMMIT_MASK               (1 << 22)
#define IA32E_CPUID7_0_B_CLFLUSHOPT_MASK            (1 << 23)
#define IA32E_CPUID7_0_B_CLWB_MASK                  (1 << 24)
#define IA32E_CPUID7_0_B_SHA_MASK                   (1 << 29)

#define IA32E_CPUID7_0_C_REFETCHWT1_MASK            (1 << 0)
#define IA32E_CPUID7_0_C_UMIP_MASK                  (1 << 2)
#define IA32E_CPUID7_0_C_GFNI_MASK                  (1 << 8)
#define IA32E_CPUID7_0_C_VAES_MASK                  (1 << 9)
#define IA32E_CPUID7_0_C_VPCLMULQDQ_MASK            (1 << 10)
#define IA32E_CPUID7_0_C_AVX512_VNNI_MASK           (1 << 11)
#define IA32E_CPUID7_0_C_AVX512_BITALG_MASK         (1 << 12)
#define IA32E_CPUID7_0_C_AVX512_VPOPCNTDQ_MASK      (1 << 14)
#define IA32E_CPUID7_0_C_CLDEMOTE_MASK              (1 << 25)
#define IA32E_CPUID7_0_C_MOVDIRI_MASK               (1 << 27)
#define IA32E_CPUID7_0_C_MOVDIR64B_MASK              (1 << 28)

#define IA32E_CPUID7_0_D_FSRM_MASK                  (1 << 4)
#define IA32E_CPUID7_0_D_AVX512_VP2INTERSECT_MASK   (1 << 8)
#define IA32E_CPUID7_0_D_RTM_ABORT_MASK             (1 << 11)
#define IA32E_CPUID7_0_D_SERIALIZE_MASK             (1 << 14)
#define IA32E_CPUID7_0_D_TSXLDTRK_MASK              (1 << 16)
#define IA32E_CPUID7_0_D_AVX512_FP16_MASK           (1 << 8)
#define IA32E_CPUID7_0_D_ARCH_CAP_MASK              (1 << 29)

#define IA32E_CPUID7_1_A_SM3_MASK                   (1 << 1)
#define IA32E_CPUID7_1_A_SM4_MASK                   (1 << 2)
#define IA32E_CPUID7_1_A_RAO_INT_MASK               (1 << 3)
#define IA32E_CPUID7_1_A_AVX_VNNI_MASK              (1 << 4)
#define IA32E_CPUID7_1_A_AVX512_BF16_MASK           (1 << 5)
#define IA32E_CPUID7_1_A_CMPCCXADD_MASK             (1 << 7)
#define IA32E_CPUID7_1_A_FZRM_MASK                  (1 << 10)
#define IA32E_CPUID7_1_A_FSRS_MASK                  (1 << 11)
#define IA32E_CPUID7_1_A_RSRCS_MASK                 (1 << 12)
#define IA32E_CPUID7_1_A_LKGS_MASK                  (1 << 18)
#define IA32E_CPUID7_1_A_WRMSRNS_MASK               (1 << 19)
#define IA32E_CPUID7_1_A_AVX_IFMA_MASK              (1 << 23)
#define IA32E_CPUID7_1_A_BIOS_DONE_MASK             (1 << 30)
#define IA32E_CPUID7_1_A_MOVRS_MASK                 (1 << 31)

#define IA32E_CPUID7_1_C_MSR_IMM_MASK               (1 << 5)

#define IA32E_CPUID7_1_D_AVX512_VNNI_FP16_MASK      (1 << 1)
#define IA32E_CPUID7_1_D_AVX512_VNNI_INT8_MASK      (1 << 2)
#define IA32E_CPUID7_1_D_AVX512_NE_CONVERT_MASK     (1 << 3)
#define IA32E_CPUID7_1_D_AVX_VNNI_INT8_MASK         (1 << 4)
#define IA32E_CPUID7_1_D_AVX_NE_CONVERT_MASK        (1 << 5)
#define IA32E_CPUID7_1_D_AVX_VNNI_INT16_MASK        (1 << 10)
#define IA32E_CPUID7_1_D_AVX512_VNNI_INT16_MASK     (1 << 11)
#define IA32E_CPUID7_1_D_PREFETCHI_MASK             (1 << 14)
#define IA32E_CPUID7_1_D_AVX512_BF16_NE_MASK        (1 << 16)
#define IA32E_CPUID7_1_D_AVX10_MASK                 (1 << 19)

#define IA32E_CPUID18_0_A_SGX1_MASK         (1 << 0)

#define IA32E_CPUID_ESIG0 0x80000000

#define IA32E_CPUID_ESIG1 0x80000001
#define IA32E_CPUID_ESIG1_C_LAHF64_MASK     (1 << 0)
#define IA32E_CPUID_ESIG1_C_LZCNT_MASK      (1 << 5)
#define IA32E_CPUID_ESIG1_C_PREFETCHW_MASK  (1 << 8)
#define IA32E_CPUID_ESIG1_D_SYSCALL_MASK    (1 << 11)
#define IA32E_CPUID_ESIG1_D_NX_MASK         (1 << 20)
#define IA32E_CPUID_ESIG1_D_PG_1GB_MASK     (1 << 26)
#define IA32E_CPUID_ESIG1_D_RDTSCP_MASK     (1 << 27)
#define IA32E_CPUID_ESIG1_D_INTEL64_MASK    (1 << 29)


#define IA32E_CPUID_ESIG2 0x80000002
#define IA32E_CPUID_ESIG3 0x80000003
#define IA32E_CPUID_ESIG4 0x80000004
#define IA32E_CPUID_ESIG5 0x80000005
#define IA32E_CPUID_ESIG6 0x80000006

#define IA32E_CPUID_ESIG7 0x80000007
#define IA32E_CPUID_ESIG7_D_INVARIANT_TSC_MASK  (1 << 8)

#define IA32E_CPUID_ESIG8 0x80000008
#define IA32E_CPUID_ESIG8_B_WBNOINVD_MASK       (1 << 9)

#define IA32E_CPUID_CORETYPE 0x1A

#define IA32E_CORETYPE_ECORE 0x20
#define IA32E_CORETYPE_PCORE 0x40

#define IA32E_INVAL 0
#define IA32E_SMT   1
#define IA32E_CORE  2

/* Apic */

#define IA32E_XAPIC_PHYS_REG_BASE           0xFEE00000

#define IA32E_XAPIC_ID_OFFSET               0x20
#define IA32E_XAPIC_VERSION_OFFSET          0x30
#define IA32E_XAPIC_TPR_OFFSET              0x80
#define IA32E_XAPIC_APR_OFFSET              0x90
#define IA32E_XAPIC_PPR_OFFSET              0xa0
#define IA32E_XAPIC_EOI_OFFSET              0xb0
#define IA32E_XAPIC_RRD_OFFSET              0xc0
#define IA32E_XAPIC_LDR_OFFSET              0xd0
#define IA32E_XAPIC_DFR_OFFSET              0xe0
#define IA32E_XAPIC_SIVR_OFFSET             0xf0
#define IA32E_XAPIC_ISR_OFFSET              0x100
#define IA32E_XAPIC_TMR_OFFSET              0x180
#define IA32E_XAPIC_IRR_OFFSET              0x200
#define IA32E_XAPIC_ESR_OFFSET              0x280
#define IA32E_XAPIC_CMCI_OFFSET             0x2f0
#define IA32E_XAPIC_ICR_LOW_OFFSET          0x300
#define IA32E_XAPIC_ICR_HIGH_OFFSET         0x310
#define IA32E_XAPIC_TIMER_OFFSET            0x320
#define IA32E_XAPIC_TSR_OFFSET              0x330
#define IA32E_XAPIC_PMCR_OFFSET             0x340
#define IA32E_XAPIC_LINT0_OFFSET            0x350
#define IA32E_XAPIC_LINT1_OFFSET            0x360
#define IA32E_XAPIC_LVT_ERROR_OFFSET        0x370
#define IA32E_XAPIC_INITIAL_COUNT_OFFSET    0x380
#define IA32E_XAPIC_CUR_COUNT_OFFSET        0x390
#define IA32E_XAPIC_DCR_OFFSET              0x3e0

#define IA32E_XAPIC_DEST_PHYSICAL           0
#define IA32E_XAPIC_DEST_LOGICAL            1

#define IA32E_XAPIC_DEASSERT                0
#define IA32E_XAPIC_ASSERT                  1

#define IA32E_XAPIC_TRIGGER_EDGE            0
#define IA32E_XAPIC_TRIGGER_LEVEL           1

#define IA32E_XAPIC_SINGLE_TARGET           0
#define IA32E_XAPIC_SELF_TARGET             1
#define IA32E_XAPIC_ALL_TARGETS             2
#define IA32E_XAPIC_OTHER_TARGETS           3

#define IA32E_XAPIC_DIV_16                  0x3

#define IA32E_XAPIC_ONESHOT                 0
#define IA32E_XAPIC_PERIODIC                1
#define IA32E_XAPIC_TSC_DEADLINE            2

/* Ioapic */

#define IA32E_IOAPIC_REG_REDIRECTION_MIN    0x10
#define IA32E_IOAPIC_REG_REDIRECTION_MAX    0x3f

/* flags */

#define IA32E_FLAGS_ALWAYS1_MASK    (1 << 1)
#define IA32E_FLAGS_TF_MASK         (1 << 8)
#define IA32E_FLAGS_IF_MASK         (1 << 9)
#define IA32E_FLAGS_DF_MASK         (1 << 10)
#define IA32E_FLAGS_VM_MASK         (1 << 17)
#define IA32E_FLAGS_AC_MASK         (1 << 18)

/* Segmentation */

#define IA32E_ACCESS_RIGHTS_DPL_MASK        (3 << 5)
#define IA32E_ACCESS_RIGHTS_LONGMODE_MASK   (1 << 13)
#define IA32E_ACCESS_RIGHTS_DB_MASK         (1 << 14)

/* Msr */

#define IA32E_SYSENTER_CS   0x174
#define IA32E_SYSENTER_ESP  0x175
#define IA32E_SYSENTER_EIP  0x176

#define IA32E_PAT 0x277

#define IA32E_EFER                          0xC0000080
#define IA32E_EFER_SYSCALL_ENABLE_MASK      (1 << 0)
#define IA32E_EFER_LONGMODE_ENABLE_MASK     (1 << 8)
#define IA32E_EFER_LONGMODE_ACTIVE_MASK     (1 << 10)
#define IA32E_EFER_XD_ENABLE_MASK           (1 << 10)

#define IA32E_FS_BASE           0xC0000100
#define IA32E_GS_BASE           0xC0000101
#define IA32E_KERNEL_GS_BASE    0xC0000102

#define IA32E_STAR      0xC0000081
#define IA32E_LSTAR     0xC0000082
#define IA32E_CSTAR     0xC0000083
#define IA32E_FMASK     0xC0000084

#define IA32E_BIOS_DONE 0x151

#define IA32E_APIC_BASE                     0x1b
#define IA32E_APIC_BASE_BSP_MASK            (1 << 8)
#define IA32E_APIC_BASE_ENABLE_X2APIC_MASK  (1 << 10)
#define IA32E_APIC_BASE_GLOBAL_EN_MASK  (1 << 11)

#define IA32E_X2APIC_BASE           0x800
#define IA32E_X2APIC_ID             0x802
#define IA32E_X2APIC_VERSION        0x803
#define IA32E_X2APIC_TPR            0x808
#define IA32E_X2APIC_PPR            0x80a
#define IA32E_X2APIC_EOI            0x80b
#define IA32E_X2APIC_LDR            0x80d
#define IA32E_X2APIC_SIVR           0x80f

#define IA32E_X2APIC_ISR0           0x810
#define IA32E_X2APIC_ISR1           0x811
#define IA32E_X2APIC_ISR2           0x812
#define IA32E_X2APIC_ISR3           0x813
#define IA32E_X2APIC_ISR4           0x814
#define IA32E_X2APIC_ISR5           0x815
#define IA32E_X2APIC_ISR6           0x816
#define IA32E_X2APIC_ISR7           0x817

#define IA32E_X2APIC_TMR0           0x818
#define IA32E_X2APIC_TMR1           0x819
#define IA32E_X2APIC_TMR2           0x81a
#define IA32E_X2APIC_TMR3           0x81b
#define IA32E_X2APIC_TMR4           0x81c
#define IA32E_X2APIC_TMR5           0x81d
#define IA32E_X2APIC_TMR6           0x81e
#define IA32E_X2APIC_TMR7           0x81f

#define IA32E_X2APIC_IRR0           0x820
#define IA32E_X2APIC_IRR1           0x821
#define IA32E_X2APIC_IRR2           0x822
#define IA32E_X2APIC_IRR3           0x823
#define IA32E_X2APIC_IRR4           0x824
#define IA32E_X2APIC_IRR5           0x825
#define IA32E_X2APIC_IRR6           0x826
#define IA32E_X2APIC_IRR7           0x827

#define IA32E_X2APIC_ESR            0x828

#define IA32E_X2APIC_LVT_CMCI       0x82f
#define IA32E_X2APIC_ICR            0x830

#define IA32E_X2APIC_LVT_TIMER      0x832
#define IA32E_X2APIC_LVT_TSR        0x833
#define IA32E_X2APIC_LVT_PMCR       0x834
#define IA32E_X2APIC_LVT_LINT0      0x835
#define IA32E_X2APIC_LVT_LINT1      0x836
#define IA32E_X2APIC_LVT_ERROR      0x837
#define IA32E_X2APIC_TIMER_INIT_COUNT 0x838
#define IA32E_X2APIC_TIMER_CUR_COUNT  0x839

#define IA32E_X2APIC_DIV_CONF       2110
#define IA32E_X2APIC_SELF_IPI       2111

#define IA32E_XAPIC_DISABLE_STATUS                  0xbd
#define IA32E_XAPIC_DISABLE_STATUS_DISABLED_MASK    (1 << 0)

#define IA32E_ARCH_CAP                              0x10A
#define IA32E_ARCH_CAP_XAPIC_DISABLE_STATUS_MASK    (1 << 21)

#define IA32E_VMX_BASIC                 0x480
#define IA32E_VMX_PINBASED_CTLS         0x481
#define IA32E_VMX_TRUE_PINBASED_CTLS    0x48D
#define IA32E_VMX_PROCBASED_CTLS        0x482
#define IA32E_VMX_TRUE_PROCBASED_CTLS   0x48E
#define IA32E_VMX_PROCBASED_CTLS2       0x48B
#define IA32E_VMX_PROCBASED_CTLS3       0x492
#define IA32E_VMX_EXIT_CTLS             0x483
#define IA32E_VMX_TRUE_EXIT_CTLS        0x48F
#define IA32E_VMX_EXIT_CTLS2            0x493
#define IA32E_VMX_ENTRY_CTLS            0x484
#define IA32E_VMX_TRUE_ENTRY_CTLS       0x490
#define IA32E_VMX_VMFUNC                0x491
#define IA32E_VMX_MISC                  0x485
#define IA32E_VMX_CR0_FIXED0            0x486
#define IA32E_VMX_CR0_FIXED1            0x487
#define IA32E_VMX_CR4_FIXED0            0x488
#define IA32E_VMX_CR4_FIXED1            0x489
#define IA32E_VMX_VMCS_ENUM             0x48A
#define IA32E_VMX_EPT_VPID_CAP          0x48C

#define IA32E_VMX_BASIC_PHYS_ADDR_WIDTH_BIT                     48
#define IA32E_VMX_BASIC_INFO_REPORTS_BIT                        54
#define IA32E_VMX_BASIC_TRUE_CTLS_BIT                           55

#define IA32E_VMX_EPT_VPID_CAP_PWLEN4_MASK                      (1 << 6)
#define IA32E_VMX_EPT_VPID_CAP_PAGE_UC_MASK                     (1 << 8)
#define IA32E_VMX_EPT_VPID_CAP_PAGE_WB_MASK                     (1 << 14)
#define IA32E_VMX_EPT_VPID_CAP_PAGE_2MB_MASK                    (1 << 16)
#define IA32E_VMX_EPT_VPID_CAP_PAGE_1GB_MASK                    (1 << 17)
#define IA32E_VMX_EPT_VPID_CAP_PAGE_AD_MASK                     (1 << 21)

#define IA32E_FEATURE_CONTROL                           0x03A
#define IA32E_FEATURE_CONTROL_LOCKED_MASK               (1 << 0)
#define IA32E_FEATURE_CONTROL_VMX_OUTSIDE_SMX_MASK      (1 << 2)
#define IA32E_FEATURE_CONTROL_SGX_LAUNCH_CONTROL_MASK   (1 << 17)
#define IA32E_FEATURE_CONTROL_SGX_GLOBAL_CONTROL_MASK   (1 << 18)

/* Misc */

#define IA32E_INTERRUPT_GATE64 0xe

#ifndef ASM_FILE

#include <compiler.h>

typedef uint64_t ia32ePml4e_t;
typedef uint64_t ia32ePdpte_t;
typedef uint64_t ia32ePde_t;
typedef uint64_t ia32ePte_t;

typedef struct ia32ePml4
{
    ia32ePml4e_t pml4e[512];
} ia32ePml4_t;
SIZE_ASSERT(ia32ePml4_t, 4096);

typedef struct ATTR_PACKED ia32eTss64 
{
    uint32_t reserved0;
    uint64_t rsp0;               
    uint64_t rsp1;               
    uint64_t rsp2;               
    uint64_t reserved1;          
    uint64_t ist1;               
    uint64_t ist2;               
    uint64_t ist3;               
    uint64_t ist4;               
    uint64_t ist5;               
    uint64_t ist6;             
    uint64_t ist7;               
    uint64_t reserved2;          
    uint16_t reserved3;
    uint16_t iopbBase;   
} ia32eTss64_t;
SIZE_ASSERT(ia32eTss64_t, 104);

typedef struct ATTR_PACKED ia32eTssFull64
{
    ia32eTss64_t tss;
    uint8_t iopb[8193];
} ia32eTssFull64_t;
SIZE_ASSERT(ia32eTssFull64_t, sizeof(ia32eTss64_t) + 8193);

typedef struct ATTR_PACKED ia32eIdtDescriptor64
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved0;
} ia32eIdtDescriptor64_t;
SIZE_ASSERT(ia32eIdtDescriptor64_t, 16);

typedef struct ATTR_PACKED ia32eDescriptorReg64
{
    uint16_t limit;
    uint64_t base;
} ia32eDescriptorReg64_t;
SIZE_ASSERT(ia32eDescriptorReg64_t, 10);

typedef struct ATTR_PACKED ia32eFxsave64 
{
    uint16_t fcw;
    uint16_t fsw;
    uint8_t ftw;
    uint8_t reserved1;
    uint16_t fop;
    uint64_t fip;
    uint64_t fdp;
    uint32_t mxcsr;
    uint32_t mxcsrMask;

    struct 
    {
        uint64_t mantissa;
        uint16_t exponent;
        uint8_t reserved[6];
    } st_mm[8];

    struct 
    {
        uint64_t low;
        uint64_t high;
    } xmm[16];

    uint8_t reserved2[96];
} ia32eFxsave64_t;
SIZE_ASSERT(ia32eFxsave64_t, 512);

typedef enum
{
    IA32E_DIVIDE_ERROR =                    0,
    IA32E_DEBUG_EXCEPTION =                 1,
    IA32E_NMI =                             2,
    IA32E_BREAKPOINT =                      3,
    IA32E_OVERFLOW =                        4,
    IA32E_BOUND_RANGE_EXCEEDED =            5,
    IA32E_INVALID_OPCODE =                  6,
    IA32E_DEVICE_NOT_AVAILABLE =            7,
    IA32E_DOUBLE_FAULT =                    8,
    IA32E_COPROCESSOR_SEGMENT_OVERRUN =     9,
    IA32E_INVALID_TSS =                     10,
    IA32E_SEGMENT_NOT_PRESENT =             11,
    IA32E_STACK_SEGMENT_FAULT =             12,
    IA32E_GENERAL_PROTECTION_FAULT =        13,
    IA32E_PAGE_FAULT =                      14,
    IA32E_VECTOR15 =                        15,
    IA32E_MATH_FAULT =                      16,
    IA32E_ALIGNMENT_CHECK =                 17,
    IA32E_MACHINE_CHECK =                   18,
    IA32E_SIMD_FLOATING_POINT_EXCEPTION =   19,
    IA32E_VIRTUALISATION_EXCEPTION =        20,
    IA32E_CONTROL_PROTECTION_EXCEPTION =    21,
    IA32E_VECTOR22 =                        22,
    IA32E_VECTOR23 =                        23,
    IA32E_VECTOR24 =                        24,
    IA32E_VECTOR25 =                        25,
    IA32E_VECTOR26 =                        26,
    IA32E_VECTOR27 =                        27,
    IA32E_VECTOR28 =                        28,
    IA32E_VECTOR29 =                        29,
    IA32E_VECTOR30 =                        30,
    IA32E_VECTOR31 =                        31,
    IA32E_EXTERNAL_INTERRUPT_MIN =          32,
    IA32E_EXTERNAL_INTERRUPT_MAX =          255
} ia32eVector_t;

typedef enum 
{
    IA32E_DM_NORMAL =           0,
    IA32E_DM_LOW_PRIORITY =     1,
    IA32E_DM_SMI =              2,
    IA32E_DM_NMI =              4,
    IA32E_DM_INIT =             5,
    IA32E_DM_STARTUP =          6,
    IA32E_DM_EXTERNAL =         7
} ia32eDeliveryMode_t;

typedef enum
{
	IA32E_INTERRUPT_TYPE_EXTERNAL =                 0,
	IA32E_INTERRUPT_TYPE_RESERVED =                 1,
	IA32E_INTERRUPT_TYPE_NMI =                      2,
	IA32E_INTERRUPT_TYPE_HARDWARE_EXCEPTION =       3,
	IA32E_INTERRUPT_TYPE_SOFTWARE_INT =             4,
	IA32E_INTERRUPT_TYPE_PRIV_SOFTWARE_EXCEPTION =  5,
	IA32E_INTERRUPT_TYPE_SOFTWARE_EXCEPTION =       6,
	IA32E_INTERRUPT_TYPE_OTHER_EVENT =              7
} ia32eInterruptType_t;

typedef struct ATTR_PACKED ia32eIoapic
{
    uint32_t reg;
    uint32_t pad[3];
    uint32_t data;
} ia32eIoapic_t;
SIZE_ASSERT(ia32eIoapic_t, 20);

#define ia32eIoapicRegLow(pin) (IA32E_IOAPIC_REG_REDIRECTION_MIN + ((pin) * 2))
#define ia32eIoapicRegHigh(pin) (ia32eIoapicRegLow((pin)) + 1)

#endif

#endif