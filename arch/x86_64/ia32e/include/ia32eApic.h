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

#ifndef _IA32E_APIC_H_
#define _IA32E_APIC_H_

#include <generated/autoconf.h>
#include <compiler.h>
#include <ia32eAsm.h>

#define IA32E_PIC1_COMMAND  0x20
#define IA32E_PIC1_DATA     0x21
#define IA32E_PIC2_COMMAND  0xA0
#define IA32E_PIC2_DATA     0xA1

inline
uint32_t ia32eIoapicRead(volatile ia32eIoapic_t *mmio, uint32_t reg)
{
    uint32_t val = 0;

    mmio->reg = reg;
    barrier();

    val = mmio->data;
    barrier();

    return val;
}

inline
void ia32eIoapicWrite(volatile ia32eIoapic_t *mmio, uint32_t reg, uint32_t val)
{
    mmio->reg = reg;
    barrier();
    
    mmio->data = val;
    barrier();
}

void ia32eMask8259(void);

void ia32eApicMmioInit(void);

uint64_t ia32eApicRead(uint32_t offset, bool xapicRead64);
void ia32eApicWrite(uint32_t offset, uint64_t val, bool xapicWrite64);

void ia32eApicConfigMadtNmiOverrides(void);

ia32eIoapic_t *ia32eIoapicGsiToMmio(uint32_t gsi, uint32_t *gsiBase);
void ia32eIoapicConfigMadtNmiOverrides(void);

void ia32eApApicSync(void);

void ia32eApicDisable(void);
void ia32eApicEnable(uint8_t spuriousVector);

uint32_t ia32eApicCalibrate(uint8_t spuriousVector);
uint32_t ia32eApicFrequencyHz(uint8_t spuriousVector);

void ia32eApicWaitForDelivery(void);
void ia32eApicSendIpi(uint32_t apicId, uint8_t vector, uint32_t deliveryMode, uint32_t destMode, uint32_t destType);
void ia32eApicWakeup(uint32_t apicId, uint8_t vector);

void ia32eApicEoi(void);

bool ia32eApicCheckIrr(uint8_t vector);
bool ia32eApicCheckIsr(uint8_t vector);

#endif