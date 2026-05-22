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

#include <generated/autoconf.h>
#include <ia32eUart.h>
#include <ia32eCpu.h>
#include <ia32eVma.h>
#include <ia32eApic.h>
#include <ia32eHpet.h>
#include <ia32eHostCpu.h>
#include <workhorse/kInit/kInit.h>
#include <lib/multiboot2.h>
#include <stdWorkhorse.h>
#include <lib/acpi.h>
#include <export/kCpuInterface.h>
#include <export/kTimerInterface.h>
#include <export/kCallbackInterface.h>

static 
kCpuOps_t cpuOps = {
    .kThisCpuIdFn = ia32eThisCpuId,
    .kCpuInvokeAllRendezvousFn = ia32eCpuInvokeAllRendezvous,
    .kCpuSelfIpiFn = ia32eCpuSelfIpi,
    .kCpuTaskIdleCtxInitFn = ia32eCpuTaskIdleCtxInit,
    .kCpuTaskCtxInitFn = ia32eCpuTaskCtxInit,
    .kCpuTaskSaveCtxFn = ia32eCpuTaskSaveCtx,
    .kCpuTaskRestoreCtxFn = ia32eCpuTaskRestoreCtx,
    .kCpuSyscallGetReturnAddressFn = ia32eCpuSyscallGetReturnAddress,
    .kCpuSyscallSetReturnAddressFn = ia32eCpuSyscallSetReturnAddress,
    .kCpuExceptionSetReturnAddressFn = ia32eCpuExceptionSetReturnAddress,
    .kCpuEnterDomainFn = ia32eCpuEnterDomain,
    .kCpuTaskLsrPushFn = ia32eCpuTaskLsrPush,
    .kCpuEventSenderFn = ia32eEventSender,
    .kCpuIdValidateFn = ia32eCpuIdValidate,
    .kCpuThreadInfoInitFn = ia32eCpuThreadInfoInit,
    .kCpuLsrInfoInitFn = ia32eCpuLsrInfoInit,
    .kCpuDomainInfoInitFn = ia32eCpuDomainInfoInit
};

static 
kTimerOps_t timerOps = {
    .kTimerFrequencyHzFn = ia32eTimerFrequencyHz,
    .kTimerArmPeriodicFn = ia32eTimerArmPeriodic
};

static 
kCallbackOps_t callbackOps = {
    .kCallbackActivationFn = ia32eCallbackActivation,
    .kCallbackResponseFn = ia32eCallbackResponse,
    .kCallbackCpuHandoffFn = ia32eCallbackCpuHandoff
};

static 
void ia32eMultiboot2Parser(void)
{
    uintptr_t mb2Phys = 0;
    void *dummyPage = NULL;
    uint8_t *mb2Ptr = NULL;
    uint32_t mb2Size = 0;
    ia32eGlobal_t *global = NULL;
    struct multiboot_tag *tag = NULL;
    struct multiboot_tag_old_acpi *oldAcpi = NULL;
    acpiRsdp_t *acpiRsdp = NULL;
    bool acpiFound = false;

    if (ia32eSignature != MULTIBOOT2_BOOTLOADER_MAGIC) {
        ia32eEarlyKpanic("Unsupported bootloader\n");
        UNREACHABLE();
    }

    mb2Phys = ia32eMultiboot2Ptr;

    dummyPage = ia32eVmaEarlyMapRange(mb2Phys, 4, false); 
    if (!dummyPage) {
        ia32eEarlyKpanic("Failed to map mb2\n");
        UNREACHABLE();
    }

    mb2Size = *(uint32_t *)dummyPage;

    mb2Ptr = ia32eVmaEarlyMapRange(mb2Phys, mb2Size, false);
    if (!mb2Ptr) {
        ia32eEarlyKpanic("Failed to map mb2\n");
        UNREACHABLE();
    }

    global = ia32eGetGlobalPtr();

    global->dummyPage = (uintptr_t)dummyPage & ~(IA32E_PAGE_SIZE_2MB - 1);

    global->multiboot2.multiboot2Phys = mb2Phys;
    global->multiboot2.multiboot2Ptr = (uintptr_t)mb2Ptr;
    global->multiboot2.multiboot2Size = mb2Size;
    
    mb2Ptr += MULTIBOOT_INFO_ALIGN;
    tag = (void *)mb2Ptr;

    while (tag->type != MULTIBOOT_TAG_TYPE_END) {

        if (tag->type == MULTIBOOT_TAG_TYPE_ACPI_OLD) {

            oldAcpi = (void *)tag;
            acpiRsdp = (void *)oldAcpi->rsdp;
            global->acpi.rsdtPhys = acpiRsdp->rsdtAddr;
            acpiFound = true;
            break;
        }

        mb2Ptr += (tag->size + (MULTIBOOT_INFO_ALIGN - 1)) & ~(MULTIBOOT_INFO_ALIGN - 1);
        tag = (void *)mb2Ptr;
    }

    if (!acpiFound) {
        ia32eEarlyKpanic("Failed to find acpi rsdp\n");
        UNREACHABLE();
    }
}

static 
void ia32eRsdtParser(void)
{
    ia32eGlobal_t *global = NULL;

    uintptr_t dummyPage = 0;

    uintptr_t rsdtPhys = 0;
    uintptr_t rsdtPhysPgBase = 0;
    uint32_t rsdtOffset = 0;
    
    acpiRsdt_t *rsdt = NULL;
    size_t rsdtSize = 0;
    
    uint32_t numEntries = 0;
    
    uint32_t i = 0;

    uint32_t hdrPhys = 0;
    uintptr_t hdrPhysPgBase = 0;
    uint32_t hdrOffset = 0;
    acpiSdtHdr_t *hdr = NULL;
    size_t hdrSize = 0;
    uint32_t hdrSig = 0;

    bool madtFound = false;
    size_t madtSize = 0;
    uintptr_t madtPhys = 0;

    bool fadtFound = false;
    size_t fadtSize = 0;
    uintptr_t fadtPhys = 0;

    bool hpetFound = false;
    size_t hpetSize = 0;
    uintptr_t  hpetPhys = 0;

    bool acpiPmFound = false;
    bool acpiPm = false;

    void *madt = NULL;
    void *fadt = NULL;
    void *acpiPmMmio = NULL;
    void *hpet = NULL;

    acpiFadt_t *acpiFadt = NULL;

    global = ia32eGetGlobalPtr();

    dummyPage = global->dummyPage;

    rsdtPhys = global->acpi.rsdtPhys;
    rsdtPhysPgBase = rsdtPhys & ~(IA32E_PAGE_SIZE_2MB - 1);
    rsdtOffset = rsdtPhys - rsdtPhysPgBase;

    ia32eVmaEarlyRemapPg(dummyPage, rsdtPhysPgBase, false);
    rsdt = (void *)(dummyPage + rsdtOffset);

    rsdtSize = rsdt->hdr.length;
    numEntries = (rsdtSize - sizeof(acpiSdtHdr_t)) / sizeof(uint32_t);

    rsdt = ia32eVmaEarlyMapRange(rsdtPhys, rsdtSize, false);
    if (!rsdt) {
        ia32eEarlyKpanic("Failed to map rsdt\n");
        UNREACHABLE();
    }

    for (i = 0; i < numEntries; i++) {

        hdrPhys = rsdt->entries[i];
        hdrPhysPgBase = hdrPhys & ~(IA32E_PAGE_SIZE_2MB - 1);
        hdrOffset = hdrPhys - hdrPhysPgBase;

        ia32eVmaEarlyRemapPg(dummyPage, hdrPhysPgBase, false);

        hdr = (void *)(dummyPage + hdrOffset);
        hdrSize = hdr->length;
        hdrSig = ACPI_SIG32(hdr->signature);

        switch (hdrSig) {

            case ACPI_MADT_SIGNATURE_UINT32:

                madtFound = true;
                madtSize = hdrSize;
                madtPhys = hdrPhys;
                break;

            case ACPI_FADT_SIGNATURE_UINT32:

                fadtFound = true;
                fadtSize = hdrSize;
                fadtPhys = hdrPhys;                
                break;

            case ACPI_HPET_SIGNATURE_UINT32:

                hpetFound = true;
                hpetSize = hdrSize;
                hpetPhys = hdrPhys;
                break;

            default:
                break;
        }

        if (madtFound && fadtFound && hpetFound)
            break;
    }

    if (!madtFound) {
        ia32eEarlyKpanic("failed to find madt\n");
        UNREACHABLE();
    }

    madt = ia32eVmaEarlyMapRange(madtPhys, madtSize, false);
    if (!madt) {
        ia32eEarlyKpanic("failed to map madt\n");
        UNREACHABLE();
    }

    if (fadtFound) {
     
        fadt = ia32eVmaEarlyMapRange(fadtPhys, fadtSize, false);
        if (fadt) {

            acpiFadt = fadt;
            if (acpiFadt->pmTmrLen == 4) {

                acpiPmFound = true;

                if (acpiFadt->hdr.revision >= 2 && acpiFadt->xPmTmrBlk.address != 0) {

                    if (acpiFadt->xPmTmrBlk.addressSpaceId == ACPI_AS_ID_SYS_MEM) {

                        acpiPmMmio = ia32eVmaEarlyMapRange(acpiFadt->xPmTmrBlk.address, 4, true);
                        if (acpiPmMmio) {
                            global->acpiPm.acpiPmMmio = (uintptr_t)acpiPmMmio;
                            global->acpiPm.mmio = true;
                            acpiPm = true;
                        }

                    } else {
                        global->acpiPm.acpiPmPort = acpiFadt->xPmTmrBlk.address;
                        acpiPm = true;   
                    }

                } else if (acpiFadt->pmTmrBlk != 0) {
                    global->acpiPm.acpiPmPort = acpiFadt->pmTmrBlk;
                    acpiPm = true;
                }
                
                global->acpiPm.found = acpiPm;
            }
        }
    }

    if (hpetFound) {
    
        hpet = ia32eVmaEarlyMapRange(hpetPhys, hpetSize, false);
        if (hpet) {

            global->hpet.found = true;

        } else {

            if (!acpiPmFound) {
                ia32eEarlyKpanic("failed to map hpet and acpiPM timer not found\n");
                UNREACHABLE();
            } 

            if (!acpiPm) {
                ia32eEarlyKpanic("failed to map hpet and acpiPM timer\n");
                UNREACHABLE();
            }
        }
 
    } else {

        if (!acpiPmFound) {
            ia32eEarlyKpanic("failed to find hpet and acpiPM timer not found\n");
            UNREACHABLE();
        } 

        if (!acpiPm) {
            ia32eEarlyKpanic("failed to find hpet and failed to map acpiPM timer\n");
            UNREACHABLE();
        }
    }

    global->acpi.rsdtPtr = (uintptr_t)rsdt;
    global->acpi.madtPtr = (uintptr_t)madt;
    global->acpi.fadtPtr = (uintptr_t)fadt;
    global->acpi.hpetPtr = (uintptr_t)hpet;
}

static 
void ia32eMadtParser(void)
{
    ia32eGlobal_t *global = NULL;

    acpiMadt_t *madt = NULL;
    size_t madtEntriesSize = 0;
    uint8_t *madtEntry = NULL;
    uint8_t *madtEnd = NULL;
    acpiEntryHdr_t *madtEntryHdr = NULL;

    uint32_t lapicId = 0;
    uint32_t threadId = 0;
    uint32_t coreId = 0;
    uint32_t pkgId = 0;

    acpiMadtLapic_t *typeLapic = NULL;
    acpiMadtX2Apic_t *typeX2apic = NULL;
    acpiMadtIoApic_t *typeIoapic = NULL;
    acpiMadtLapicAddrOverride_t *typeAddrOverride = NULL;

    ia32ePerCpu_t *cpu = NULL;
    uint32_t numCpus = 0;
    uint32_t bspIdx = 0;
    bool bspFound = false;

    uint32_t numIoapics = 0;

    global = ia32eGetGlobalPtr();

    madt = (void *)global->acpi.madtPtr;
    madtEntriesSize = madt->hdr.length - sizeof(acpiMadt_t);
    madtEntry = (void *)madt->entries;
    madtEnd = madtEntry + madtEntriesSize;

    global->apic.apicMmioPhys = madt->localInterruptControllerAddress;

    ia32eThisTopology(&lapicId, &threadId, &coreId, &pkgId);
    
    while (madtEntry < madtEnd) {

        madtEntryHdr = (void *)madtEntry;
    
        if (madtEntry + madtEntryHdr->length > madtEnd)
            break;

        switch (madtEntryHdr->type) {

            case ACPI_MADT_ENTRY_TYPE_LAPIC:

                typeLapic = (void *)madtEntry;
                if (numCpus == CONFIG_KMAX_CPUS) {
                    ia32eEarlyKpanic("Too many cpus, likely bad config\n");
                    UNREACHABLE();
                }

                cpu = &global->cpuTable[numCpus];

                cpu->thisPtr = cpu;
                cpu->global = global;

                cpu->cpuId = numCpus;
                cpu->apicId = typeLapic->id;
                cpu->acpiUid = typeLapic->uid;
                cpu->cpuFlags.fields.enabled = (typeLapic->flags & ACPI_MADT_LAPIC_FLAGS_ENABLED_MASK) != 0;
                cpu->cpuFlags.fields.onlineCapable = (typeLapic->flags & ACPI_MADT_LAPIC_FLAGS_ONLINE_CAP_MASK) != 0;

                if (typeLapic->id == lapicId) {

                    cpu->topology.threadId = threadId;
                    cpu->topology.coreId = coreId;
                    cpu->topology.pkgId = pkgId;

                    cpu->cpuFlags.fields.online = 1;
                    cpu->cpuFlags.fields.bsp = 1;

                    bspIdx = numCpus;
                    bspFound = true;
                }

                numCpus++;
                break;

            case ACPI_MADT_ENTRY_TYPE_LOCAL_X2APIC:

                typeX2apic = (void *)madtEntry;
                if (numCpus == CONFIG_KMAX_CPUS) {
                    ia32eEarlyKpanic("Too many cpus, likely bad config\n");
                    UNREACHABLE();
                }

                cpu = &global->cpuTable[numCpus];

                cpu->thisPtr = cpu;
                cpu->global = global;

                cpu->cpuId = numCpus;
                cpu->apicId = typeX2apic->id;
                cpu->acpiUid = typeX2apic->uid;
                cpu->cpuFlags.fields.enabled = (typeX2apic->flags & ACPI_MADT_LAPIC_FLAGS_ENABLED_MASK) != 0;
                cpu->cpuFlags.fields.onlineCapable = (typeX2apic->flags & ACPI_MADT_LAPIC_FLAGS_ONLINE_CAP_MASK) != 0;

                if (typeX2apic->id == lapicId) {

                    cpu->topology.threadId = threadId;
                    cpu->topology.coreId = coreId;
                    cpu->topology.pkgId = pkgId;

                    cpu->cpuFlags.fields.online = 1;
                    cpu->cpuFlags.fields.bsp = 1;

                    bspIdx = numCpus;
                    bspFound = true;
                }

                numCpus++;
                break;

            case ACPI_MADT_ENTRY_TYPE_IOAPIC:

                typeIoapic = (void *)madtEntry;

                if (numIoapics == CONFIG_IA32E_MAX_IOAPICS) {
                    ia32eEarlyKpanic("Too many ioapics, likely bad config\n");
                    UNREACHABLE();
                }

                global->ioapic.ioapicData[numIoapics].ioapicMmioPhys = typeIoapic->address;
                global->ioapic.ioapicData[numIoapics].ioapicGsiBase = typeIoapic->gsiBase;

                numIoapics++;
                break;

            case ACPI_MADT_ENTRY_TYPE_LAPIC_ADDRESS_OVERRIDE:

                typeAddrOverride = (void *)madtEntry;
                global->apic.apicMmioPhys = typeAddrOverride->address;
                break;
        }

        madtEntry += madtEntryHdr->length;
    }

    if (!bspFound) {
        ia32eEarlyKpanic("Bad acpi madt, couldn't locate bsp\n");
        UNREACHABLE();
    }

    global->numCpus = numCpus;
    global->bsp = bspIdx;

    global->ioapic.numIoApics = numIoapics;

    __ia32eWrmsr(IA32E_GS_BASE, (uintptr_t)&global->cpuTable[bspIdx]);
}

static 
void ia32eHpetParser(void)
{
    ia32eGlobal_t *global = NULL;
    acpiHpet_t *hpet = NULL;
    uintptr_t hpetMmioPhys = 0;
    void *hpetMmio = NULL;

    global = ia32eThisCpuData()->global;
    if (!global->hpet.found)
        return;

    hpet = (void *)global->acpi.hpetPtr;

    hpetMmioPhys = hpet->address.address;

    /* Hpet spec defines 4KB max */

    hpetMmio = ia32eVmaEarlyMapRange(hpetMmioPhys, IA32E_PAGE_SIZE_4KB, true);
    if (!hpetMmio) {
        ia32eEarlyKpanic("failed to map hpet mmio\n");
        UNREACHABLE();
    }

    global->hpet.hpetMmio = (uintptr_t)hpetMmio;
    ia32eHpetMmioInit(hpetMmio);
}

static 
void ia32eApicConfig(void)
{
    ia32eGlobal_t *global = NULL;
    uint32_t regs[4] = {0};
    uint64_t base = 0;
    uint64_t archCap = 0;
    uint64_t xapicDisableStatus = 0;
    uintptr_t apicMmioPhys = 0;
    uint64_t newBase = 0;
    void *apicMmioPtr = NULL;

    global = ia32eThisCpuData()->global;

    ia32eCpuid(1, 0, &regs[0], &regs[1], &regs[2], &regs[3]);

    base = __ia32eRdmsr(IA32E_APIC_BASE);

    if ((regs[2] & IA32E_CPUID1_C_X2APIC_MASK) != 0) {

        if ((base & IA32E_APIC_BASE_GLOBAL_ENABLE_MASK) == 0 || (base & IA32E_APIC_BASE_ENABLE_X2APIC_MASK) == 0) {

            base |= IA32E_APIC_BASE_GLOBAL_ENABLE_MASK;
            base |= IA32E_APIC_BASE_ENABLE_X2APIC_MASK;

            __ia32eWrmsr(IA32E_APIC_BASE, base);
        }

        global->gFlags.fields.x2apic = 1;
        
    } else {

        ia32eCpuid(7, 0, &regs[0], &regs[1], &regs[2], &regs[3]);

        if ((regs[3] & IA32E_CPUID7_0_D_ARCH_CAP_MASK) != 0) {

            archCap = __ia32eRdmsr(IA32E_ARCH_CAP);

            if ((archCap & IA32E_ARCH_CAP_XAPIC_DISABLE_STATUS_MASK) != 0) {

                xapicDisableStatus = __ia32eRdmsr(IA32E_XAPIC_DISABLE_STATUS);

                if ((xapicDisableStatus & IA32E_XAPIC_DISABLE_STATUS_DISABLED_MASK) != 0) {
                    ia32eEarlyKpanic("xapic disabled, likely bad firmware or interrupt controller\n");
                    UNREACHABLE();
                }
            }
        }

        apicMmioPhys = global->apic.apicMmioPhys;
        if ((apicMmioPhys & 0xfff) != 0) {
            ia32eEarlyKpanic("apic mmio region not 4kb aligned, likely bad firmware\n");
            UNREACHABLE();
        }

        newBase = apicMmioPhys | IA32E_APIC_BASE_GLOBAL_ENABLE_MASK;
        if (base != newBase)
            __ia32eWrmsr(IA32E_APIC_BASE, newBase);
        
        apicMmioPtr = ia32eVmaEarlyMapRange(apicMmioPhys, IA32E_PAGE_SIZE_4KB, true);
        if (!apicMmioPtr) {
            ia32eEarlyKpanic("failed to map apic mmio\n");
            UNREACHABLE();
        }

        global->apic.apicMmio = (uintptr_t)apicMmioPtr;
    }

    ia32eApicMmioInit();
    ia32eApicDisable();
}

static
void ia32eIoapicConfig(void)
{
    ia32eGlobal_t *global = NULL;
    uint32_t numIoapics = 0;
    uint32_t i = 0;
    void *ioapicMmio = NULL;

    global = ia32eThisCpuData()->global;

    numIoapics = global->ioapic.numIoApics;

    for (i = 0; i < numIoapics; i++) {

        ioapicMmio = ia32eVmaEarlyMapRange(global->ioapic.ioapicData[i].ioapicMmioPhys, IA32E_PAGE_SIZE_4KB, true);
        if (!ioapicMmio) {
            ia32eEarlyKpanic("failed to map ioapic mmio\n");
            UNREACHABLE();
        }

        global->ioapic.ioapicData[i].ioapicMmio = (uintptr_t)ioapicMmio;
    }
}

static
void ia32eApWakeup(void)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eGlobal_t *global = NULL;
    uint32_t thisApicId = 0;
    uint32_t numCpus = 0;
    char *wakeupArea = NULL;
    size_t wakeupBlobSize = 0;

    uint32_t i = 0;
    ia32ePerCpu_t *apCpu = NULL;
    uint32_t apApicId = 0;
    uint32_t wokenCount = 0;

    cpu = ia32eThisCpuData();
    global = cpu->global;
    thisApicId = cpu->apicId;
    numCpus = global->numCpus;
    wakeupArea = (void *)IA32E_WAKEUP_ADDR;
    wakeupBlobSize = ia32eWakeupBlobEnd - ia32eWakeupBlobStart;
    
    memcpy(ia32eWakeupBlobSaveArea, wakeupArea, wakeupBlobSize);
    memcpy(wakeupArea, ia32eWakeupBlobStart, wakeupBlobSize);

    ia32eHpetEnableCounter();

    for (i = 0; i < numCpus; i++) {

        apCpu = &global->cpuTable[i];
        apApicId = apCpu->apicId;
        if (apApicId == thisApicId || apCpu->cpuFlags.fields.enabled == 0)
            continue;

        ia32eApRsp = (uintptr_t)&apCpu->wakeupStack.stack[sizeof(apCpu->wakeupStack.stack)];
        ia32eApDr0 = (uintptr_t)apCpu->wakeupStack.padding;

        ia32eApicWakeup(apApicId, IA32E_WAKEUP_VECTOR);

        wokenCount++;
        spinUntil(atomic_load(&global->numCpusOnline) == wokenCount);
    }

    memcpy(wakeupArea, ia32eWakeupBlobSaveArea, wakeupBlobSize);
}

static
void ia32eBspConfig(void)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eGlobal_t *global = NULL;

    cpu = ia32eThisCpuData();
    global = cpu->global;

    ia32eTssInit();
    ia32eGdtInit();
    ia32eTrInit();
    ia32eIdtInit();

    ia32eApicEnable(IA32E_SPURIOUS_INT_VECTOR);
    cpu->apicFrequencyHz = ia32eApicFrequencyHz(IA32E_SPURIOUS_INT_VECTOR);
    cpu->mxcsrMask = ia32eMxcsrMask64();
    ia32eCpuInit();

    ia32eHpetDisableCounter();
    atomic_fetch_add(&global->numCpusOnline, 1);

    global->ipiData.ipiSender = ia32eThisCpuData()->cpuId;
    global->intcSetup = true;

#if CONFIG_IA32E_VTX
    ia32eGlobalVtxInit();
#endif
}

static
void ia32eFinalizeHighMap(void)
{
    ia32ePml4e_t *ia32ePml4eReloc = NULL;

    ia32ePml4eReloc = (void *)((uintptr_t)ia32ePml4 + IA32E_KERNEL_OFFSET);
    ia32ePml4eReloc[0] = 0;
    barrier();

    ia32eFlushTlbAll();
}

static
void ia32eInterfacesInit(void)
{
    kCpuOpsInit(&cpuOps);
    kTimerOpsInit(&timerOps);
    kCallbackOpsInit(&callbackOps);
}

/* Register initcalls */

K_REGISTER_INITCALL(ia32eMask8259, ia32eMask8259,                                           000);
K_REGISTER_INITCALL(ia32eUartInit, ia32eUartInit,                                           001);
K_REGISTER_INITCALL(ia32eEarlyIdtInit, ia32eEarlyIdtInit,                                   002);
K_REGISTER_INITCALL(ia32eVmaInit, ia32eVmaInit,                                             003);
K_REGISTER_INITCALL(ia32eMultiboot2Parser, ia32eMultiboot2Parser,                           004);
K_REGISTER_INITCALL(ia32eRsdtParser, ia32eRsdtParser,                                       005);
K_REGISTER_INITCALL(ia32eMadtParser, ia32eMadtParser,                                       006);
K_REGISTER_INITCALL(ia32eHpetParser, ia32eHpetParser,                                       007);
K_REGISTER_INITCALL(ia32eApicConfig, ia32eApicConfig,                                       008);
K_REGISTER_INITCALL(ia32eIoapicConfig, ia32eIoapicConfig,                                   009);

#if CONFIG_IA32E_APPLY_MADT_NMI_OVERRIDES

K_REGISTER_INITCALL(ia32eApicConfigMadtNmiOverrides, ia32eApicConfigMadtNmiOverrides,       010);
K_REGISTER_INITCALL(ia32eIoapicConfigMadtNmiOverrides, ia32eIoapicConfigMadtNmiOverrides,   011);

#endif

K_REGISTER_INITCALL(ia32eApWakeup, ia32eApWakeup,                                           012);
K_REGISTER_INITCALL(ia32eBspConfig, ia32eBspConfig,                                         013);
K_REGISTER_INITCALL(ia32eFinalizeHighMap, ia32eFinalizeHighMap,                             014);
K_REGISTER_INITCALL(ia32eInterfacesInit, ia32eInterfacesInit,                               015);