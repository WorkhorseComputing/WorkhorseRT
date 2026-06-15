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

#ifndef _IA32E_HPET_H_
#define _IA32E_HPET_H_

#include <ia32eAsm.h>

#define IA32E_HPET_CAPABILITIES_OFFSET      0
#define IA32E_HPET_GCR_OFFSET               0x10
#define IA32E_HPET_GISR_OFFSET              0x20
#define IA32E_HPET_MCVR_OFFSET              0xf0
#define IA32E_HPET_TIMER0_CONFIG_OFFSET     0x100
#define IA32E_HPET_TIMER0_COMPARATOR_OFFSET 0x108
#define IA32E_HPET_TIMER0_FSB_OFFSET        0x110

#define IA32E_HPET_CAP_NUM_TIMERS(cap)      (((cap) >> 8) & 0x1f)
#define IA32E_HPET_CAP_PERIOD_FS(cap)       ((cap) >> 32)

#define IA32E_HPET_GCR_ENABLE_CNF_MASK  (1 << 0)
#define IA32E_HPET_GCR_LEG_RT_CNF_MASK  (1 << 1)

#define IA32E_HPET_GISR_TN_INT_STS_MASK 0xffffffff

#define IA32E_HPET_TIMER_TN_INT_ENABLE_MASK (1 << 2)

#define ia32eHpetTimerConfigOffset(timerNo) (IA32E_HPET_TIMER0_CONFIG_OFFSET + (timerNo * 32))
#define ia32eHpetTimerComparatorOffset(timerNo) (IA32E_HPET_TIMER0_COMPARATOR_OFFSET + (timerNo * 32))
#define ia32eHpetTimerFsbOffset(timerNo) (HPET_TIMER0_FSB_OFFSET + (timerNo * 32))

uint64_t ia32eHpetMmioRead(uint32_t offset);
void ia32eHpetMmioWrite(uint32_t offset, uint64_t val);

void ia32eHpetEnableCounter(void);
void ia32eHpetDisableCounter(void);
void ia32eHpetClearCounter(void);
uint32_t ia32eHpetReadCounter(void);

void ia32eHpetDisableTimer(uint32_t timerNo);

void ia32eHpetMmioInit(void *hpetMmioPtr);

bool ia32eHpetIsInitialized(void);
uint32_t ia32eHpetFrequencyHz(void);

#endif