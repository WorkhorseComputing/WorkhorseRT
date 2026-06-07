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

#include <ia32eApic.h>

#include <ia32eCpu.h>
#include <ia32eHpet.h>
#include <lib/acpi.h>
#include <stdWorkhorse.h>

static 
volatile 
uint8_t *apicMmio = NULL;

static 
bool x2apic = false;

void ia32eMask8259(void)
{
    __ia32eOutb(IA32E_PIC1_COMMAND, 0x11);
    __ia32eOutb(IA32E_PIC2_COMMAND, 0x11);
    
    __ia32eOutb(IA32E_PIC1_DATA, 0x20);
    __ia32eOutb(IA32E_PIC2_DATA, 0x28);

    __ia32eOutb(IA32E_PIC1_DATA, 0x4);
    __ia32eOutb(IA32E_PIC2_DATA, 0x2);

    __ia32eOutb(IA32E_PIC1_DATA, 0x1);
    __ia32eOutb(IA32E_PIC2_DATA, 0x1);

    __ia32eOutb(IA32E_PIC1_DATA, 0xff);
    __ia32eOutb(IA32E_PIC2_DATA, 0xff);
}

void ia32eApicMmioInit(void)
{
    ia32eGlobal_t *global = NULL;

    global = ia32eThisCpuData()->global;

    apicMmio = (void *)global->apic.apicMmio;
    x2apic = global->gFlags.fields.x2apic != 0;
}

uint64_t ia32eApicRead(uint32_t offset, bool xapicRead64)
{
    uint64_t val = 0;
    uint64_t extended = 0;

    if (!x2apic) {

        if (offset == IA32E_XAPIC_ICR_LOW_OFFSET && xapicRead64) {

            val =  READ_ONCE(*(uint32_t *)(apicMmio + offset));
            extended = READ_ONCE(*(uint32_t *)(apicMmio + IA32E_XAPIC_ICR_HIGH_OFFSET));
         
            return val | (extended << 32);
        }

        val =  READ_ONCE(*(uint32_t *)(apicMmio + offset));
        return val;
    }

    return __ia32eRdmsr(IA32E_X2APIC_BASE + (offset / 16));
}

void ia32eApicWrite(uint32_t offset, uint64_t val, bool xapicWrite64)
{
    if (!x2apic) {

        if (offset == IA32E_XAPIC_ICR_LOW_OFFSET && xapicWrite64)
           WRITE_ONCE(*(uint32_t *)(apicMmio + IA32E_XAPIC_ICR_HIGH_OFFSET), val >> 32);

        WRITE_ONCE(*(uint32_t *)(apicMmio + offset), val & 0xffffffff);
        return;
    }

    __ia32eWrmsr(IA32E_X2APIC_BASE + (offset / 16), val);
}

void ia32eApicConfigMadtNmiOverrides(void)
{
    ia32ePerCpu_t *cpu = NULL;
    ia32eGlobal_t *global = NULL;
    uint32_t uid = 0;

    acpiMadt_t *madt = NULL;
    size_t madtEntriesSize = 0;
    uint8_t *madtEntry = NULL;
    uint8_t *madtEnd = NULL;
    acpiEntryHdr_t *madtEntryHdr = NULL;

    acpiMadtLapicNmi_t *lapicNmi = NULL;
    acpiMadtX2ApicNmi_t *x2apicNmi = NULL;

    uint32_t polarity = 0;
    uint32_t triggerMode = 0;

    bool activeLow = false;
    bool levelTriggered = false;

    uint32_t lintOffset = 0;
    uint32_t lintVal = 0;

    cpu = ia32eThisCpuData();
    global = cpu->global;
    uid = cpu->acpiUid;

    madt = (void *)global->acpi.madtPtr;
    madtEntriesSize = madt->hdr.length - sizeof(acpiMadt_t);
    madtEntry = (void *)madt->entries;
    madtEnd = madtEntry + madtEntriesSize;

    while (madtEntry < madtEnd) {

        madtEntryHdr = (void *)madtEntry;
    
        if (madtEntry + madtEntryHdr->length > madtEnd)
            break;

        switch (madtEntryHdr->type) {

            case ACPI_MADT_ENTRY_TYPE_LAPIC_NMI:
                
                lapicNmi = (void *)madtEntryHdr;

                if (lapicNmi->uid != uid && lapicNmi->uid != 0xff)
                    break;

                polarity = lapicNmi->flags & ACPI_MADT_POLARITY_MASK;
                triggerMode = lapicNmi->flags & ACPI_MADT_TRIGGERING_MASK;

                activeLow = polarity == ACPI_MADT_POLARITY_ACTIVE_LOW;
                levelTriggered = triggerMode == ACPI_MADT_TRIGGERING_LEVEL;

                if (lapicNmi->lint == 0)
                    lintOffset = IA32E_XAPIC_LINT0_OFFSET;
                else if (lapicNmi->lint == 1)
                    lintOffset = IA32E_XAPIC_LINT1_OFFSET;
                else
                    break;

                lintVal = (IA32E_NMI) | (IA32E_DM_NMI << 8) | (activeLow << 13) | (levelTriggered << 15);
                ia32eApicWrite(lintOffset, lintVal, false);
                break;

            case ACPI_MADT_ENTRY_TYPE_LOCAL_X2APIC_NMI:

                x2apicNmi = (void *)madtEntryHdr;

                if (x2apicNmi->uid != uid && x2apicNmi->uid != 0xffffffff)
                    break;

                polarity = x2apicNmi->flags & ACPI_MADT_POLARITY_MASK;
                triggerMode = x2apicNmi->flags & ACPI_MADT_TRIGGERING_MASK;

                activeLow = polarity == ACPI_MADT_POLARITY_ACTIVE_LOW;
                levelTriggered = triggerMode == ACPI_MADT_TRIGGERING_LEVEL;

                if (x2apicNmi->lint == 0)
                    lintOffset = IA32E_XAPIC_LINT0_OFFSET;
                else if (x2apicNmi->lint == 1)
                    lintOffset = IA32E_XAPIC_LINT1_OFFSET;
                else
                    break;

                lintVal = (IA32E_NMI) | (IA32E_DM_NMI << 8) | (activeLow << 13) | (levelTriggered << 15);
                ia32eApicWrite(lintOffset, lintVal, false); 
                break;

            default:
                break;

        }

        madtEntry += madtEntryHdr->length;
    }
}

ia32eIoapic_t *ia32eIoapicGsiToMmio(uint32_t gsi, uint32_t *gsiBase)
{
    ia32eGlobal_t *global = NULL;

    uint32_t numIoapics = 0;
    uint32_t retGsiBase = 0;
    ia32eIoapic_t *ioapicMmio = NULL;
    uint32_t i = 0;
    uint32_t curGsiBase = 0;

    global = ia32eThisCpuData()->global;

    numIoapics = global->ioapic.numIoApics;

    for (i = 0; i < numIoapics; i++) {

        curGsiBase = global->ioapic.ioapicData[i].ioapicGsiBase;
        if (curGsiBase >= retGsiBase && curGsiBase <= gsi) {
            retGsiBase = curGsiBase;
            ioapicMmio = (void *)global->ioapic.ioapicData[i].ioapicMmio;
        }
    }

    if (gsiBase)
        *gsiBase = retGsiBase;

    return ioapicMmio;
}

void ia32eIoapicConfigMadtNmiOverrides(void)
{
    ia32eGlobal_t *global = NULL;

    acpiMadt_t *madt = NULL;
    size_t madtEntriesSize = 0;
    uint8_t *madtEntry = NULL;
    uint8_t *madtEnd = NULL;
    acpiEntryHdr_t *madtEntryHdr = NULL;

    acpiMadtNmiSource_t *nmiSource = NULL;

    uint32_t gsi = 0;
    uint32_t polarity = 0;
    uint32_t triggerMode = 0;

    bool activeLow = false;
    bool levelTriggered = false;

    uint32_t gsiBase = 0;
    volatile ia32eIoapic_t *ioapicMmio = NULL;
    uint32_t pin = 0;
    uint32_t regLow = 0;
    uint32_t regHigh = 0;
    uint32_t entryLow = 0;
    uint32_t entryHigh = 0;

    global = ia32eThisCpuData()->global;

    madt = (void *)global->acpi.madtPtr;
    madtEntriesSize = madt->hdr.length - sizeof(acpiMadt_t);
    madtEntry = (void *)madt->entries;
    madtEnd = madtEntry + madtEntriesSize;

    entryHigh = global->gFlags.fields.x2apic != 0 ? 0x7fff << 17: 0xff << 24;

    while (madtEntry < madtEnd) {

        madtEntryHdr = (void *)madtEntry;
    
        if (madtEntry + madtEntryHdr->length > madtEnd)
            break;

        switch (madtEntryHdr->type) {

            case ACPI_MADT_ENTRY_TYPE_NMI_SOURCE:

                nmiSource = (void *)madtEntryHdr;

                gsi = nmiSource->gsi;
                polarity = nmiSource->flags & ACPI_MADT_POLARITY_MASK;
                triggerMode = nmiSource->flags & ACPI_MADT_TRIGGERING_MASK;
                activeLow = polarity == ACPI_MADT_POLARITY_ACTIVE_LOW;
                levelTriggered = triggerMode == ACPI_MADT_TRIGGERING_LEVEL;

                ioapicMmio = ia32eIoapicGsiToMmio(gsi, &gsiBase);
                if (!ioapicMmio)
                    break;

                pin = gsi - gsiBase;
                regLow = ia32eIoapicRegLow(pin);
                regHigh = ia32eIoapicRegHigh(pin);
                entryLow = (IA32E_NMI) | (IA32E_DM_NMI << 8) | (activeLow << 13) | (levelTriggered << 15);
                
                ia32eIoapicWrite(ioapicMmio, regLow, entryLow);
                ia32eIoapicWrite(ioapicMmio, regHigh, entryHigh);
                break;

            default:
                break;
        }

        madtEntry += madtEntryHdr->length;
    }
}

void ia32eApApicSync(void)
{
    ia32eGlobal_t *global = NULL;
    uint32_t regs[4] = {0};

    uint64_t base = 0;

    uint64_t archCap = 0;
    uint64_t xapicDisableStatus = 0;

    uintptr_t apicMmioPhys = 0;
    uint64_t newBase = 0;

    ia32eCpuid(1, 0, &regs[0], &regs[1], &regs[2], &regs[3]);
        
    if (((regs[2] & IA32E_CPUID1_C_X2APIC_MASK) != 0) != x2apic) {
        ia32eEarlyKpanic("likely broken apic, some cores have x2apic, some xapic\n");
        UNREACHABLE();
    }
    
    base = __ia32eRdmsr(IA32E_APIC_BASE);

    if (x2apic) {

        if ((base & IA32E_APIC_BASE_GLOBAL_EN_MASK) == 0 || (base & IA32E_APIC_BASE_ENABLE_X2APIC_MASK) == 0) {

            base |= IA32E_APIC_BASE_GLOBAL_EN_MASK;
            base |= IA32E_APIC_BASE_ENABLE_X2APIC_MASK;

            __ia32eWrmsr(IA32E_APIC_BASE, base);
        }
        
        return;
    }

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

    global = ia32eThisCpuData()->global;

    apicMmioPhys = global->apic.apicMmioPhys;
    newBase = apicMmioPhys | IA32E_APIC_BASE_GLOBAL_EN_MASK;
    if (base != newBase)
        __ia32eWrmsr(IA32E_APIC_BASE, newBase);
}

void ia32eApicDisable(void)
{
    uint32_t sivr = 0;
    
    sivr = ia32eApicRead(IA32E_XAPIC_SIVR_OFFSET, false);
    sivr &= ~(1 << 8);
    ia32eApicWrite(IA32E_XAPIC_SIVR_OFFSET, sivr, false);
}

void ia32eApicEnable(uint8_t spuriousVector)
{
    uint32_t sivr = 0;
    
    sivr = ia32eApicRead(IA32E_XAPIC_SIVR_OFFSET, false);
    sivr |= ((uint32_t)spuriousVector) | (1 << 8);
    ia32eApicWrite(IA32E_XAPIC_SIVR_OFFSET, sivr, false);
}

uint32_t ia32eApicCalibrate(uint8_t spuriousVector)
{
    ia32eGlobal_t *global = NULL;    

    volatile uint32_t *acpiPmMmio = NULL;
    uint16_t acpiPmPort = 0;

    uint32_t timer = 0;

    uint32_t counterFrequencyHz = 0;
    uint64_t calibrationTicks = 0;
    uint64_t startTicks = 0;
    
    uint32_t apicCur = 0;
    uint32_t apicTicksElapsed = 0;
    uint32_t apicFrequencyHz = 0;

    global = ia32eThisCpuData()->global;

    timer = spuriousVector | (1 << 16) | (IA32E_XAPIC_ONESHOT << 17);
    ia32eApicWrite(IA32E_XAPIC_DCR_OFFSET, IA32E_XAPIC_DIV_16, false);
    ia32eApicWrite(IA32E_XAPIC_TIMER_OFFSET, timer, false);
    ia32eApicWrite(IA32E_XAPIC_INITIAL_COUNT_OFFSET, UINT32_MAX, false);

    if (ia32eHpetIsInitialized()) {

        counterFrequencyHz = ia32eHpetFrequencyHz();
        calibrationTicks = msToTicks(CONFIG_IA32E_APIC_CALIBRATION_TIME_MS, counterFrequencyHz);
        startTicks = ia32eHpetReadCounter();
        spinUntil(ia32eHpetReadCounter() - startTicks >= calibrationTicks);

    } else {

        counterFrequencyHz = ACPI_PM_TMR_HZ;
        calibrationTicks = msToTicks(CONFIG_IA32E_APIC_CALIBRATION_TIME_MS, counterFrequencyHz);

        if (global->acpiPm.mmio) {

            acpiPmMmio = (void *)global->acpiPm.acpiPmMmio;
            startTicks = READ_ONCE(*acpiPmMmio);
            spinUntil(READ_ONCE(*acpiPmMmio) - startTicks >= calibrationTicks);

        } else {
            acpiPmPort = (uint16_t)global->acpiPm.acpiPmPort;
            startTicks = __ia32eInl(acpiPmPort);
            spinUntil( __ia32eInl(acpiPmPort) - startTicks >= calibrationTicks);
        }
    }

    apicCur = ia32eApicRead(IA32E_XAPIC_CUR_COUNT_OFFSET, false) & 0xffffffff;
    apicTicksElapsed = UINT32_MAX - apicCur;

    apicFrequencyHz = (apicTicksElapsed * counterFrequencyHz) / calibrationTicks;    

    if (apicFrequencyHz == 0) {
        ia32eEarlyKpanic("failed to calibrate apic, ended up with a frequency of 0 after manual calibration\n");
        UNREACHABLE();
    }

    return apicFrequencyHz;
}

uint32_t ia32eApicFrequencyHz(uint8_t spuriousVector)
{
    uint32_t regs[4] = {0};

    ia32eCpuid(0x15, 0, &regs[0], &regs[1], &regs[2], &regs[3]);

    if (regs[0] != 0 && regs[1] != 0 && regs[2] != 0)
        return regs[2] / IA32E_XAPIC_DIV_16;

    return ia32eApicCalibrate(spuriousVector);
}

void ia32eApicWaitForDelivery(void)
{
    uint64_t icrLow = 0;

    if (ia32eThisCpuData()->global->gFlags.fields.x2apic != 0)
        return;

    do {
        icrLow = ia32eApicRead(IA32E_XAPIC_ICR_LOW_OFFSET, false);
        cpuRelax();
    } while ((icrLow & (1 << 12)) != 0);
}

void ia32eApicSendIpi(uint32_t apicId, uint8_t vector, uint32_t deliveryMode, uint32_t destMode, uint32_t destType)
{
    ia32eApicWaitForDelivery();

    uint64_t icrLow = 0;
    uint64_t dest = 0;
    uint64_t icr = 0;

    icrLow = vector | (deliveryMode << 8) | (destMode << 11) | (IA32E_XAPIC_ASSERT << 14) | (destType << 18);
    dest = apicId;

    if (!x2apic)
        dest <<= 24;

    icr = icrLow | (dest << 32);

    ia32eApicWrite(IA32E_XAPIC_ICR_LOW_OFFSET, icr, true);
    ia32eApicWaitForDelivery();
}

void ia32eApicWakeup(uint32_t apicId, uint8_t vector)
{
    uint64_t icrLow = 0;
    uint64_t dest = 0;
    uint64_t icr = 0;

    icrLow = (IA32E_DM_INIT << 8) | (IA32E_XAPIC_ASSERT << 14);
    dest = apicId;

    if (!x2apic) {
        icrLow |= (IA32E_XAPIC_TRIGGER_LEVEL << 15);
        dest <<= 24;
    }

    icr = icrLow | (dest << 32);

    ia32eApicWrite(IA32E_XAPIC_ICR_LOW_OFFSET, icr, true);
    ia32eApicWaitForDelivery();

    if (!x2apic) {

        icrLow &= ~(IA32E_XAPIC_ASSERT << 14);
        ia32eApicWrite(IA32E_XAPIC_ICR_LOW_OFFSET, icrLow, false);       
        ia32eApicWaitForDelivery();
    }

    ia32eApicSendIpi(apicId, vector, IA32E_DM_STARTUP, IA32E_XAPIC_DEST_PHYSICAL, IA32E_XAPIC_SINGLE_TARGET);
    ia32eApicSendIpi(apicId, vector, IA32E_DM_STARTUP, IA32E_XAPIC_DEST_PHYSICAL, IA32E_XAPIC_SINGLE_TARGET);
}

void ia32eApicEoi(void)
{
    ia32eApicWrite(IA32E_XAPIC_EOI_OFFSET, 0, false);
}

bool ia32eApicCheckIrr(uint8_t vector)
{
    uint8_t idx = 0;
    uint8_t bit = 0;
    uint32_t offset = 0;

    idx = vector / 32;
    bit = vector % 32;
    offset = IA32E_XAPIC_IRR_OFFSET + (idx * 16);

    return ((ia32eApicRead(offset, false) >> bit) & 1) != 0;
}

bool ia32eApicCheckIsr(uint8_t vector)
{
    uint8_t idx = 0;
    uint8_t bit = 0;
    uint32_t offset = 0;

    idx = vector / 32;
    bit = vector % 32;
    offset = IA32E_XAPIC_ISR_OFFSET + (idx * 16);

    return ((ia32eApicRead(offset, false) >> bit) & 1) != 0;
}