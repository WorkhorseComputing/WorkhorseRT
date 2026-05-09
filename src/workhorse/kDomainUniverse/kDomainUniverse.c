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

#include <workhorse/kDomainUniverse/kDomainUniverse.h>
#include <workhorse/kTick/kTick.h>
#include <export/kCpuInterface.h>
#include <export/kDbgInterface.h>
#include <errno.h>

static
kDomainUniverse_t gDomainUniverse;

kDomain_t *kDomainUniverseGet(uint32_t domId)
{
    if (domId >= ARRAY_LEN(gDomainUniverse.domainTable) || !gDomainUniverse.domainTable[domId])
        return NULL;

    return gDomainUniverse.domainTable[domId];
}

int kDomainUniverseAdd(uint32_t domId, kDomain_t *domain)
{
    if (domId >= ARRAY_LEN(gDomainUniverse.domainTable) || gDomainUniverse.domainTable[domId])
        return -EINVAL;

    gDomainUniverse.domainTable[domId] = domain;
    domain->domId = domId;
    return 0; 
}

bool kDomainUniverseCanAdd(uint32_t domId)
{
    return domId < ARRAY_LEN(gDomainUniverse.domainTable) && !gDomainUniverse.domainTable[domId];
}

bool kDomainUniverseAuthenticateInvocation(uint32_t domId, kDomain_t *invokingDomain)
{   
    return domId < ARRAY_LEN(gDomainUniverse.domainTable) && 
            gDomainUniverse.domainTable[domId] && 
            gDomainUniverse.domainTable[domId]->invocationInfo.invocationIpc.valid && 
            BITMAP_TEST(invokingDomain->invocationInfo.invokePermMap, domId);
}

int kDomainPushInvocationEntry(uintptr_t *newPc, kDomain_t *invokedDomain, kDomainInvocationType_t type, 
                               uintptr_t returnAddress, uintptr_t vmemFaultAddress, uintptr_t errorCode)
{
    kSchedTask_t *runningTask = NULL;
    kDomain_t *invokingDomain = NULL;
    uint32_t depth = 0;
    uintptr_t newPcLocal = 0;
    bool valid = false;

    K_DYNAMIC_ASSERT(newPc);

    runningTask = kTickGetRunningTask();
    invokingDomain = runningTask->domain.curDomain;
    depth = runningTask->domain.invocationStackDepth;

    K_DYNAMIC_ASSERT(depth <= ARRAY_LEN(runningTask->domain.invocationStack));

    if (depth == ARRAY_LEN(runningTask->domain.invocationStack))
        return type == K_INVOCATION_IPC ? -EINVAL : -EFAULT;

    switch (type) {

        case K_INVOCATION_IPC:
            newPcLocal = invokedDomain->invocationInfo.invocationIpc._entry;
            valid = invokedDomain->invocationInfo.invocationIpc.valid;
            break;

        case K_INVOCATION_EXCEPTION_VMEM_FAULT:
            newPcLocal = invokedDomain->invocationInfo.invocationExceptionVmemFault._entry;
            valid = invokedDomain->invocationInfo.invocationExceptionVmemFault.valid;
            break;

        case K_INVOCATION_EXCEPTION_ILLEGAL_OPCODE:
            newPcLocal = invokedDomain->invocationInfo.invocationExceptionIllegalOpcode._entry;
            valid = invokedDomain->invocationInfo.invocationExceptionIllegalOpcode.valid;
            break;

        case K_INVOCATION_EXCEPTION_ALIGNMENT:
            newPcLocal = invokedDomain->invocationInfo.invocationExceptionAlignment._entry;
            valid = invokedDomain->invocationInfo.invocationExceptionAlignment.valid;
            break;

        case K_INVOCATION_EXCEPTION_DEBUG:
            newPcLocal = invokedDomain->invocationInfo.invocationExceptionDebug._entry;
            valid = invokedDomain->invocationInfo.invocationExceptionDebug.valid;
            break;

        case K_INVOCATION_EXCEPTION_ARITHMETIC:
            newPcLocal = invokedDomain->invocationInfo.invocationExceptionArithmetic._entry;
            valid = invokedDomain->invocationInfo.invocationExceptionArithmetic.valid;
            break;

        case K_INVOCATION_EXCEPTION_OTHER:
            newPcLocal = invokedDomain->invocationInfo.invocationExceptionOther._entry;
            valid = invokedDomain->invocationInfo.invocationExceptionOther.valid;
            break;

        default:
            break;
    }

    if (!valid)
        return -EINVAL;

    runningTask->domain.curDomain = invokedDomain;

    runningTask->domain.invocationStack[depth].invokingDomain = invokingDomain;
    runningTask->domain.invocationStack[depth].type = type;
    runningTask->domain.invocationStack[depth].returnAddress = returnAddress;
    runningTask->domain.invocationStack[depth].vmemFaultAddress = vmemFaultAddress;
    runningTask->domain.invocationStack[depth].errorCode = errorCode;

    runningTask->domain.invocationStackDepth++;

    *newPc = newPcLocal;

    return 0;
}

int kDomainPopInvocationEntry(uintptr_t *newPc, kDomain_t **newDomain)
{
    kSchedTask_t *task = NULL;
    uint32_t depth = 0;
    uintptr_t newPcLocal = 0;
    kDomain_t *newDomainLocal = NULL;

    K_DYNAMIC_ASSERT(newPc);
    K_DYNAMIC_ASSERT(newDomain);

    task = kTickGetRunningTask();
    depth = task->domain.invocationStackDepth;

    if (depth == 0)
        return -EINVAL;

    depth--;

    newPcLocal = task->domain.invocationStack[depth].returnAddress;
    newDomainLocal = task->domain.invocationStack[depth].invokingDomain;
    task->domain.invocationStack[depth].type = K_INVOCATION_INVALID;

    task->domain.invocationStackDepth = depth;
    task->domain.curDomain = newDomainLocal;

    *newPc = newPcLocal;
    *newDomain = newDomainLocal;
    
    return 0;
}

kDomainInvocationEntry_t *kDomainReadInvocationEntry(void)
{
    kSchedTask_t *task = NULL;
    uint32_t depth = 0;

    task = kTickGetRunningTask();
    depth = task->domain.invocationStackDepth;
    
    if (depth == 0)
        return NULL;

    depth--;

    return &task->domain.invocationStack[depth];
}