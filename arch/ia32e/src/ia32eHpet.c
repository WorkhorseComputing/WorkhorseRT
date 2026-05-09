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

#include <ia32eHpet.h>
#include <ia32eCpu.h>
#include <stdWorkhorse.h>

static 
volatile 
uint8_t *hpetMmio = NULL;

static 
uint64_t hpetFrequencyHz = 0;

static 
bool hpetCounterOn = false;

static 
bool hpetInitialized = false;

uint64_t ia32eHpetMmioRead(uint32_t offset)
{
    return READ_ONCE(*(uint64_t *)(hpetMmio + offset));
}

void ia32eHpetMmioWrite(uint32_t offset, uint64_t val)
{
    WRITE_ONCE(*(uint64_t *)(hpetMmio + offset) , val);
}

void ia32eHpetEnableCounter(void)
{
    uint64_t gcr = 0;

    gcr = ia32eHpetMmioRead(IA32E_HPET_GCR_OFFSET);
    gcr |= IA32E_HPET_GCR_ENABLE_CNF_MASK;
    ia32eHpetMmioWrite(IA32E_HPET_GCR_OFFSET, gcr);

    hpetCounterOn = true;
}

void ia32eHpetDisableCounter(void)
{
    uint64_t gcr = 0;

    gcr = ia32eHpetMmioRead(IA32E_HPET_GCR_OFFSET);
    gcr &= ~IA32E_HPET_GCR_ENABLE_CNF_MASK;
    gcr &= ~IA32E_HPET_GCR_LEG_RT_CNF_MASK;
    ia32eHpetMmioWrite(IA32E_HPET_GCR_OFFSET, gcr);

    hpetCounterOn = false;
}

void ia32eHpetClearCounter(void)
{
    ia32eHpetMmioWrite(IA32E_HPET_MCVR_OFFSET, 0);
}

uint32_t ia32eHpetReadCounter(void)
{
    return ia32eHpetMmioRead(IA32E_HPET_MCVR_OFFSET);
}

void ia32eHpetDisableTimer(uint32_t timerNo)
{
    uint32_t configOffset = 0;
    uint32_t comparatorOffset = 0;
    uint32_t config = 0;

    configOffset = ia32eHpetTimerConfigOffset(timerNo);
    comparatorOffset = ia32eHpetTimerComparatorOffset(timerNo); 

    config = ia32eHpetMmioRead(configOffset);
    config &= ~IA32E_HPET_TIMER_TN_INT_ENABLE_MASK;
    ia32eHpetMmioWrite(configOffset, config);

    ia32eHpetMmioWrite(comparatorOffset, -1ULL);
}

void ia32eHpetMmioInit(void *hpetMmioPtr)
{
    uint64_t hpetCap = 0;
    uint32_t numTimers = 0;
    uint32_t counterPeriodFs = 0;
    uint32_t i = 0;
    uint64_t gisr = 0;

    hpetMmio = hpetMmioPtr;

    hpetCap = ia32eHpetMmioRead(IA32E_HPET_CAPABILITIES_OFFSET);

    numTimers = IA32E_HPET_CAP_NUM_TIMERS(hpetCap);
    counterPeriodFs = IA32E_HPET_CAP_PERIOD_FS(hpetCap);
    
    if (counterPeriodFs == 0)
        return;

    hpetFrequencyHz = FEMTOSECOND / counterPeriodFs;

    if (hpetFrequencyHz == 0)
        return;

    ia32eHpetDisableCounter();
    ia32eHpetClearCounter();

    for (i = 0; i < numTimers; i++)
        ia32eHpetDisableTimer(i);

    gisr = ia32eHpetMmioRead(IA32E_HPET_GISR_OFFSET);
    if ((gisr & IA32E_HPET_GISR_TN_INT_STS_MASK) != 0)
        ia32eHpetMmioWrite(IA32E_HPET_GISR_OFFSET, gisr);    

    hpetInitialized = true;
}

bool ia32eHpetIsInitialized(void)
{
    return hpetInitialized;
}

uint32_t ia32eHpetFrequencyHz(void)
{
    return hpetFrequencyHz;
}