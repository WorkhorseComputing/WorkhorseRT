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

#include <workhorse/kInit/kInit.h>
#include <workhorse/kSched/kSchedPolicy/kSchedRr.h>
#include <workhorse/kSched/kSchedPolicy/kSchedCyclic.h>
#include <workhorse/kSched/kSchedPolicy/kSchedEdf.h>
#include <workhorse/kTick/kTick.h>
#include <export/kCpuInterface.h>
#include <export/kTimerInterface.h>
#include <export/kCallbackInterface.h>
#include <plugin/kPlugin.h>
#include <stdWorkhorse.h>

static
void kPerCpuHandoff(void)
{
    uint64_t timerFreqHz = 0;
    uint64_t ticks = 0;

    kTickReschedule();

#if CONFIG_KTICK_DESYNCHRONIZED

    timerFreqHz = kTimerFrequencyHz(); 
    ticks = msToTicks(CONFIG_KTICK_MS, timerFreqHz);

    kTimerArmPeriodic(ticks);

#else

    if (kThisCpuId() == kCpuEventSender()) {
        
        timerFreqHz = kTimerFrequencyHz(); 
        ticks = msToTicks(CONFIG_KTICK_MS, timerFreqHz);

        kTimerArmPeriodic(ticks);
    }

#endif

    kCallbackCpuHandoff();
}

static 
void kHandoff(void)
{
    kCpuInvokeAllRendezvous(kPerCpuHandoff);
}

ATTR_NORETURN
void kStart(void)
{   
    uint32_t i = 0;
    uint32_t num = 0;

    num = kPluginCount();

    kInitStart();

#if CONFIG_KSCHED_POLICY_RR
    kSchedOpsInitRr();
#endif

#if CONFIG_KSCHED_POLICY_CYCLIC
    kSchedOpsInitCyclic();
#endif

#if CONFIG_KSCHED_POLICY_EDF
    kSchedOpsInitEdf();
#endif

    kCpuInvokeAllRendezvous(kSchedInit);

    for (i = 0; i < num; i++)
        __kPluginStart[i].fn();    

    gPluginsDone = true;
    barrier();
    
    kCpuSelfIpi(kHandoff);

    UNREACHABLE();
}