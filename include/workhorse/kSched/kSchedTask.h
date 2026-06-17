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

#ifndef _K_SCHED_TASK_H_
#define _K_SCHED_TASK_H_

#include <generated/autoconf.h>
#include <defs.h>
#include <workhorse/kSched/kSchedPolicy/kSchedRrDefs.h>
#include <workhorse/kSched/kSchedPolicy/kSchedCyclicDefs.h>
#include <workhorse/kSched/kSchedPolicy/kSchedEdfDefs.h>
#include <workhorse/kDomainUniverse/kDomainUniverse.h>
#include <lib/dsa/deltaChain.h>
#include <lib/dsa/dq.h>
#include <lib/dsa/stackq.h>

typedef enum kSchedTaskType
{
    K_TASK_INVALID =    0,
    K_TASK_THREAD =     1,
    K_TASK_LSR =        2,
    K_TASK_IDLE =       3
} kSchedTaskType_t;

typedef enum kSchedState
{
    K_TASK_STATE_INVALID =                      0,
    K_TASK_STATE_READY =                        1,
    K_TASK_STATE_RUNNING =                      2,
    K_TASK_STATE_THREAD_THROTTLED =             3,
    K_TASK_STATE_LSR_DORMANT =                  4,
    K_TASK_STATE_THREAD_DEFTICK_YIELD =         5,
    K_TASK_STATE_THREAD_DEFTICK_THROTTLE =      6,
    K_TASK_STATE_DEFTICK_PENDING =              7,
    K_TASK_STATE_PENDING =                      8,
    K_TASK_STATE_LSR_PENDING =                  9,
    K_TASK_STATE_FAILURE =                      10
} kSchedState_t;

#define K_SCHED_TASK_ACTIVATED(state)               \
    (!((state) == K_TASK_STATE_INVALID ||           \
    (state) == K_TASK_STATE_THREAD_THROTTLED ||     \
    (state) == K_TASK_STATE_LSR_DORMANT ||          \
    (state) == K_TASK_STATE_FAILURE))

struct kSchedTask;
typedef struct kSchedTask kSchedTask_t;

typedef void (*kSchedTaskInCallbackFn_t)(kSchedTask_t *task);
typedef void (*kSchedTaskOutCallbackFn_t)(kSchedTask_t *task);
typedef void (*kSchedTaskActivationCallbackFn_t)(kSchedTask_t *task);
typedef void (*kSchedTaskResponseCallbackFn_t)(kSchedTask_t *task);

typedef struct kSchedTick
{
    uint32_t currentBudget;
    uint32_t budget;
    uint32_t period;
    deltaNode_t replenishNode;
} kSchedTick_t;

typedef struct kSchedThread
{
    kSchedParam_t param;
    archSchedThreadInfo_t archInfo;
    kSchedThreadLink_t link;
    kSchedTick_t tick;
} kSchedThread_t;

typedef struct kSchedLsr
{
    stackqNode_t node;
    archSchedLsrInfo_t archInfo;
    kSchedLsrLink_t link;
} kSchedLsr_t;

typedef struct kSchedIdle
{
    kSchedIdleLink_t link;
} kSchedIdle_t;

typedef union kSchedTaskInfo
{
    kSchedThread_t thread;
    kSchedLsr_t lsr;
    kSchedIdle_t idle;
} kSchedTaskInfo_t;

typedef struct kSchedTaskTaggedInfo
{  
    kSchedTaskType_t type;
    kSchedTaskInfo_t info;
} kSchedTaskTaggedInfo_t;

typedef struct kSchedTaskCallbacks
{
    kSchedTaskInCallbackFn_t inCallbackFn;
    kSchedTaskOutCallbackFn_t outCallbackFn;
    kSchedTaskActivationCallbackFn_t activationCallbackFn;
    kSchedTaskResponseCallbackFn_t responseCallbackFn;
} kSchedTaskCallbacks_t;

typedef struct kSchedTask
{
    uint32_t cpuId;
    uint32_t taskId;
    kSchedState_t state;
    archSchedCtx_t ctx;

    kSchedTaskTaggedInfo_t taggedInfo;

    kSchedTaskCallbacks_t callbacks;

    struct
    {
        kDomain_t *curDomain;

        kDomainInvocationEntry_t invocationStack[CONFIG_KMAX_DOMAIN_INVOCATION_DEPTH];
        uint32_t invocationStackDepth;
    } domain;

    stackqNode_t deferredTickNode;
    
} kSchedTask_t;

inline 
kSchedTask_t *kSchedTaskFromThread(kSchedThread_t *threadPtr)
{
    kSchedTaskInfo_t *infoPtr = NULL;
    kSchedTaskTaggedInfo_t *taggedInfoPtr = NULL;
    kSchedTask_t *taskPtr = NULL;

    infoPtr = containerOf(threadPtr, kSchedTaskInfo_t, thread);
    taggedInfoPtr = containerOf(infoPtr, kSchedTaskTaggedInfo_t, info);
    taskPtr = containerOf(taggedInfoPtr, kSchedTask_t, taggedInfo);

    return taskPtr;
}

inline 
kSchedTask_t *kSchedTaskFromLsr(kSchedLsr_t *lsrPtr)
{
    kSchedTaskInfo_t *infoPtr = NULL;
    kSchedTaskTaggedInfo_t *taggedInfoPtr = NULL;
    kSchedTask_t *taskPtr = NULL;

    infoPtr = containerOf(lsrPtr, kSchedTaskInfo_t, lsr);
    taggedInfoPtr = containerOf(infoPtr, kSchedTaskTaggedInfo_t, info);
    taskPtr = containerOf(taggedInfoPtr, kSchedTask_t, taggedInfo);

    return taskPtr;
}

inline
kSchedTask_t *kSchedTaskFromThreadArchInfo(archSchedThreadInfo_t *infoPtr)
{
    kSchedThread_t *threadPtr = NULL;
    kSchedTask_t *taskPtr = NULL;

    threadPtr = containerOf(infoPtr, kSchedThread_t, archInfo);
    taskPtr = kSchedTaskFromThread(threadPtr);

    return taskPtr;
}

inline
kSchedTask_t *kSchedTaskFromLsrArchInfo(archSchedLsrInfo_t *infoPtr)
{
    kSchedLsr_t *lsrPtr = NULL;
    kSchedTask_t *taskPtr = NULL;

    lsrPtr = containerOf(infoPtr, kSchedLsr_t, archInfo);
    taskPtr = kSchedTaskFromLsr(lsrPtr);

    return taskPtr;
}

inline
kSchedTask_t *kSchedTaskFromLsrStackqNode(stackqNode_t *nodePtr)
{
    kSchedLsr_t *lsrPtr = NULL;
    kSchedTask_t *taskPtr = NULL;

    lsrPtr = containerOf(nodePtr, kSchedLsr_t, node);
    taskPtr = kSchedTaskFromLsr(lsrPtr);

    return taskPtr;
}

#endif