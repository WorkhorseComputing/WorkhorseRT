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

#include <import/kExceptionHandler.h>
#include <workhorse/kDomainUniverse/kDomainUniverse.h>
#include <workhorse/kTick/kTick.h>
#include <export/kDbgInterface.h>
#include <export/kCpuInterface.h>

void kExceptionHandler(bool fail, kDomainInvocationType_t type, uintptr_t returnAddress, 
                       uintptr_t vmemFaultAddress, uintptr_t errorCode)
{
    kSchedTask_t *runningTask = NULL;
    kDomain_t *domain = NULL;
    uintptr_t newPc = 0;

    runningTask = kTickGetRunningTask();

    K_DYNAMIC_ASSERT(runningTask);

    domain = runningTask->domain.curDomain;

    K_DYNAMIC_ASSERT(domain);

    if (!fail && kDomainPushInvocationEntry(&newPc, domain, type, returnAddress, vmemFaultAddress, errorCode) == 0)
        kCpuExceptionSetReturnAddress(newPc);
    else 
        kTickSwitchRunningTask(K_TASK_STATE_FAILURE);
}