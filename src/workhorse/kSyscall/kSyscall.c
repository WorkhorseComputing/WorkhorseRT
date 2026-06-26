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

#include <workhorse/kSyscall/kSyscall.h>
#include <workhorse/kSched/kSched.h>
#include <workhorse/kTick/kTick.h>
#include <export/kDbgInterface.h>
#include <export/kCpuInterface.h>
#include <export/kTimerInterface.h>
#include <errno.h>

static 
inline
intptr_t kInvocationCtrlDoReturn(void)
{
    kSchedTask_t *task = NULL;
    kDomain_t *curDomain = NULL;
    uintptr_t newPc = 0;
    kDomain_t *newDomain = NULL;
    int err = 0;
    
    task = kTickGetRunningTask();
    curDomain = task->domain.curDomain;

    err = kDomainPopInvocationEntry(&newPc, &newDomain);
    if (err < 0)
        return err;

    if (newDomain != curDomain)
        kCpuEnterDomain(newDomain);

    kCpuSyscallSetReturnAddress(newPc);
    return 0;
}

static
inline
intptr_t kInvocationCtrlDoIpc(uintptr_t domId)
{
    kSchedTask_t *task = NULL;
    kDomain_t *curDomain = NULL;
    uintptr_t retAddr = 0;
    uintptr_t newPc = 0;
    kDomain_t *newDomain = NULL;
    int err = 0;

    task = kTickGetRunningTask();
    curDomain = task->domain.curDomain;
    retAddr = kCpuSyscallGetReturnAddress();

    if (!kDomainUniverseAuthenticateInvocation(domId, curDomain))
        return -EINVAL;

    newDomain = kDomainUniverseGet(domId);

    err = kDomainPushInvocationEntry(&newPc, newDomain, K_INVOCATION_IPC, retAddr, 0, 0);
    if (err < 0)
        return err;

    if (newDomain != curDomain)
        kCpuEnterDomain(newDomain);

    kCpuSyscallSetReturnAddress(newPc);
    return 0;
}

static 
inline
intptr_t kInvocationCtrlGetInvocationsAvail(void)
{
    kSchedTask_t *task = NULL;
    uint32_t bound = 0;
    uint32_t depth = 0;

    task = kTickGetRunningTask();
    bound = ARRAY_LEN(task->domain.invocationStack);
    depth = task->domain.invocationStackDepth;

    K_DYNAMIC_ASSERT(bound >= depth);

    return bound - depth;
}

static 
inline 
intptr_t kInvocationCtrlGetInvokingDomId(void)
{
    kDomainInvocationEntry_t *invocationEntry = NULL;
    kDomain_t *invokingDomain = NULL;

    invocationEntry = kDomainReadInvocationEntry();
    if (!invocationEntry)
        return -EINVAL;

    invokingDomain = invocationEntry->invokingDomain;
    K_DYNAMIC_ASSERT(invokingDomain);

    return invokingDomain->domId;
}

static 
inline 
intptr_t kInvocationCtrlGetInvocationType(void)
{
    kDomainInvocationEntry_t *invocationEntry = NULL;
    intptr_t type = 0;

    invocationEntry = kDomainReadInvocationEntry();       
    if (!invocationEntry)
        return -EINVAL;

    switch (invocationEntry->type) {

        case K_INVOCATION_IPC:
            type = WORKHORSE_INVOCATION_TYPE_IPC;
            break;

        case K_INVOCATION_EXCEPTION_VMEM_FAULT:
            type = WORKHORSE_INVOCATION_TYPE_VMEM_FAULT;
            break;

        case K_INVOCATION_EXCEPTION_ILLEGAL_OPCODE:
            type = WORKHORSE_INVOCATION_TYPE_ILLEGAL_OPCODE;
            break;

        case K_INVOCATION_EXCEPTION_ALIGNMENT:
            type = WORKHORSE_INVOCATION_TYPE_ALIGNMENT;
            break;

        case K_INVOCATION_EXCEPTION_DEBUG:
            type = WORKHORSE_INVOCATION_TYPE_DEBUG;
            break;

        case K_INVOCATION_EXCEPTION_ARITHMETIC:
            type = WORKHORSE_INVOCATION_TYPE_ARITHMETIC;
            break;

        case K_INVOCATION_EXCEPTION_OTHER:
            type = WORKHORSE_INVOCATION_TYPE_OTHER;
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }

    return type;
}

static 
inline 
intptr_t kInvocationCtrlGetReturnAddress(void)
{
    kDomainInvocationEntry_t *invocationEntry = NULL;

    invocationEntry = kDomainReadInvocationEntry(); 
    if (!invocationEntry || invocationEntry->type == K_INVOCATION_IPC)
        return -EINVAL;

    return invocationEntry->returnAddress;
}

static 
inline 
intptr_t kInvocationCtrlGetVmemFaultAddress(void)
{
    kDomainInvocationEntry_t *invocationEntry = NULL;

    invocationEntry = kDomainReadInvocationEntry();       
    if (!invocationEntry || invocationEntry->type != K_INVOCATION_EXCEPTION_VMEM_FAULT)
        return -EINVAL; 
        
    return invocationEntry->vmemFaultAddress;
}

static 
inline 
intptr_t kInvocationCtrlGetErrorCode(void)
{
    kDomainInvocationEntry_t *invocationEntry = NULL;

    invocationEntry = kDomainReadInvocationEntry();       
    if (!invocationEntry || invocationEntry->type == K_INVOCATION_IPC)
        return -EINVAL;

    K_DYNAMIC_ASSERT(invocationEntry->errorCode <= INTPTR_MAX);

    return invocationEntry->errorCode;
}

static 
inline 
intptr_t kInvocationCtrlSetReturnAddress(uintptr_t returnAddress)
{
    kDomainInvocationEntry_t *invocationEntry = NULL;

    invocationEntry = kDomainReadInvocationEntry();       
    if (!invocationEntry || invocationEntry->type == K_INVOCATION_IPC)
        return -EINVAL;

    invocationEntry->returnAddress = returnAddress;
    return 0;
}

static
void kSchedCtrlYield(void)
{
    kTickSwitchRunningTask(K_TASK_STATE_THREAD_DEFTICK_YIELD);
}

static
void kSchedCtrlThrottle(void)
{
    kTickSwitchRunningTask(K_TASK_STATE_THREAD_DEFTICK_THROTTLE);
}

static
void kSchedCtrlLsrDone(void)
{
    kTickSwitchRunningTask(K_TASK_STATE_LSR_DORMANT);
}

static
void kSchedCtrlFailure(void)
{
    kTickSwitchRunningTask(K_TASK_STATE_FAILURE);
}

static
void kSchedCtrlSleepMs(void)
{
    kTickSwitchRunningTask(K_TASK_STATE_THREAD_DEFTICK_SLEEP);
}

intptr_t kSysInvocationCtrl(uintptr_t ctrl, uintptr_t val)
{
    intptr_t ret = 0;

    switch (ctrl) {

        case WORKHORSE_INVOCATION_CTRL_DO_RETURN:
            ret = kInvocationCtrlDoReturn();
            break;

        case WORKHORSE_INVOCATION_CTRL_DO_IPC:
            ret = kInvocationCtrlDoIpc(val);
            break;

        case WORKHORSE_INVOCATION_CTRL_GET_INVOCATIONS_AVAIL:
            ret = kInvocationCtrlGetInvocationsAvail();
            break;

        case WORKHORSE_INVOCATION_CTRL_GET_INVOKING_DOM_ID:
            ret = kInvocationCtrlGetInvokingDomId();
            break;

        case WORKHORSE_INVOCATION_CTRL_GET_INVOCATION_TYPE:
            ret = kInvocationCtrlGetInvocationType();
            break;

        case WORKHORSE_INVOCATION_CTRL_GET_RETURN_ADDRESS:
            ret = kInvocationCtrlGetReturnAddress();
            break;

        case WORKHORSE_INVOCATION_CTRL_GET_VMEM_FAULT_ADDRESS:
            ret = kInvocationCtrlGetVmemFaultAddress();
            break;

        case WORKHORSE_INVOCATION_CTRL_GET_ERROR_CODE:
            ret = kInvocationCtrlGetErrorCode();
            break;

        case WORKHORSE_INVOCATION_CTRL_SET_RETURN_ADDRESS:
            ret = kInvocationCtrlSetReturnAddress(val);
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

intptr_t kSysSchedCtrl(uintptr_t ctrl, uintptr_t val)
{
    intptr_t ret = 0;
    kSchedTask_t *task = NULL;
    kSchedTaskType_t type = K_TASK_INVALID;
    
    kSchedThread_t *thread = NULL;

    task = kTickGetRunningTask();
    type = task->taggedInfo.type;

    switch (ctrl) {

        case WORKHORSE_SCHED_CTRL_YIELD:

            if (type == K_TASK_THREAD)
                kCpuSelfIpi(kSchedCtrlYield);
            else
                ret = -EINVAL;

            break;

        case WORKHORSE_SCHED_CTRL_THROTTLE:

            if (type == K_TASK_THREAD)
                kCpuSelfIpi(kSchedCtrlThrottle);
            else
                ret = -EINVAL;

            break;

        case WORKHORSE_SCHED_CTRL_LSR_DONE:

            if (type == K_TASK_LSR)
                kCpuSelfIpi(kSchedCtrlLsrDone);
            else
                ret = -EINVAL;

            break;

        case WORKHORSE_SCHED_CTRL_FAILURE:

            if (type == K_TASK_THREAD || type == K_TASK_LSR)
                kCpuSelfIpi(kSchedCtrlFailure);
            else
                ret = -EINVAL;

            break;

        case WORKHORSE_SCHED_CTRL_SLEEP_MS:

            if (type != K_TASK_THREAD || (sizeof(uintptr_t) > sizeof(uint32_t) && val > UINT32_MAX) ) {
                ret = -EINVAL;
                break;
            } 

            if (val == 0)
                break;

            thread = &task->taggedInfo.info.thread;
            
            thread->tick.sleepTicks = (val + CONFIG_KTICK_MS - 1)  / CONFIG_KTICK_MS;
            kCpuSelfIpi(kSchedCtrlSleepMs);
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

intptr_t kSysGetDomId(void)
{
    kSchedTask_t *task = NULL;
    kDomain_t *domain = NULL;

    task = kTickGetRunningTask();

    K_DYNAMIC_ASSERT(task);
    
    domain = task->domain.curDomain;

    K_DYNAMIC_ASSERT(domain);

    return domain->domId;
}

intptr_t kSysGetTaskId(void)
{
    kSchedTask_t *task = NULL;

    task = kTickGetRunningTask();

    K_DYNAMIC_ASSERT(task);
    
    return task->taskId;
}

intptr_t kSysGetTaskType(void)
{
    kSchedTask_t *task = NULL;
    kSchedTaskType_t type = K_TASK_INVALID;
    intptr_t ret = 0;

    task = kTickGetRunningTask();

    K_DYNAMIC_ASSERT(task);

    type = task->taggedInfo.type;

    switch (type) {

        case K_TASK_THREAD:
            ret = WORKHORSE_TASK_TYPE_THREAD;
            break;

        case K_TASK_LSR:
            ret = WORKHORSE_TASK_TYPE_LSR;
            break;

        case K_TASK_IDLE:
            ret = WORKHORSE_TASK_TYPE_IDLE;
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            ret = -EFAULT;
            break;
    }

    return ret;
}

intptr_t kSysGetCpuId(void)
{
    intptr_t ret = 0;

#if KSYS_GET_CPU_ID
    uint64_t cpuId = 0;
    
    cpuId = kThisCpuId();

    K_DYNAMIC_ASSERT(cpuId <= INTPTR_MAX);

    ret = cpuId;
#else 
    ret = -EINVAL;
#endif

    return ret;
}