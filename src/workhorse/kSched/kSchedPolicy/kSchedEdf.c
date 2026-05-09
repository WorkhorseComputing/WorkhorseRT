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

#include <workhorse/kSched/kSchedPolicy/kSchedEdf.h>

#if CONFIG_KSCHED_POLICY_EDF

#include <workhorse/kTick/kTick.h>
#include <export/kCpuInterface.h>
#include <export/kDbgInterface.h>

static 
kSchedulerEdf_t gSchedulersEdf[CONFIG_KMAX_CPUS];

static
kSchedOps_t ops = {
    .kSchedInitFn = kSchedInitEdf,
    .kSchedTickTransitionCallbackFn = kSchedTickTransitionCallbackEdf,
    .kSchedPushFn = kSchedTaskPushEdf,
    .kSchedTaskPopFn = kSchedTaskPopEdf,
    .kSchedTaskTickCallbackFn = kSchedTaskTickCallbackEdf,
    .kSchedShouldRescheduleFn = kSchedShouldRescheduleEdf,
    .kSchedTaskLsrOutCallbackFn = kSchedTaskLsrOutCallbackEdf
};

static
inline
kSchedTask_t *kSchedTaskFromThreadDeltaNodeEdf(deltaNode_t *deltaNodePtr)
{
    kSchedThreadLinkEdf_t *linkEdfPtr = NULL;
    kSchedThreadLink_t *linkPtr = NULL;
    kSchedThread_t *threadPtr = NULL;
    kSchedTaskInfo_t *infoPtr = NULL;
    kSchedTaskTaggedInfo_t *taggedInfoPtr = NULL;
    kSchedTask_t *taskPtr = NULL;
    
    linkEdfPtr = containerOf(deltaNodePtr, kSchedThreadLinkEdf_t, node);
    linkPtr = containerOf(linkEdfPtr, kSchedThreadLink_t, linkEdf);
    threadPtr = containerOf(linkPtr, kSchedThread_t, link);
    infoPtr = containerOf(threadPtr, kSchedTaskInfo_t, thread);
    taggedInfoPtr = containerOf(infoPtr, kSchedTaskTaggedInfo_t, info);
    taskPtr = containerOf(taggedInfoPtr, kSchedTask_t, taggedInfo);

    return taskPtr;
}

static
inline 
void kSchedFixupChainEdf(void)
{
    uint32_t cpuId = 0;
    kSchedulerEdf_t *sched = NULL;

    deltaNode_t *deltaNode = NULL;
    kSchedTask_t *expiredTask = NULL;
    kSchedThread_t *expiredThread = NULL;
    uint32_t delta = 0;

    cpuId = kThisCpuId();
    sched = &gSchedulersEdf[cpuId];

    while ((deltaNode = deltaChainPopExpired(&sched->chain)) != NULL) {

        expiredTask = kSchedTaskFromThreadDeltaNodeEdf(deltaNode);
        expiredThread = &expiredTask->taggedInfo.info.thread;
        delta = expiredThread->param.paramEdf.virtualDeadline;

        deltaChainInsert(&sched->chain, deltaNode, delta);
    }
}

void kSchedOpsInitEdf(void)
{
    kSchedOpsInit(&ops);
}

void kSchedInitEdf(void)
{
    uint32_t cpuId = 0;
    kSchedulerEdf_t *sched = NULL;

    cpuId = kThisCpuId();
    sched = &gSchedulersEdf[cpuId];

    kSchedTaskIdleInit(&sched->idle);
}

void kSchedTickTransitionCallbackEdf(void)
{
    uint32_t cpuId = 0;
    kSchedulerEdf_t *sched = NULL;

    cpuId = kThisCpuId();
    sched = &gSchedulersEdf[cpuId];
   
    deltaChainTick(&sched->chain);
}

void kSchedTaskPushEdf(kSchedTask_t *task)
{
    uint32_t cpuId = 0;
    kSchedulerEdf_t *sched = NULL;
    
    kSchedTaskType_t type = K_TASK_INVALID;
    kSchedThread_t *thread = NULL;
    kSchedIdle_t *idle = NULL;

    uint32_t delta = 0;

    cpuId = task->cpuId;
    sched = &gSchedulersEdf[cpuId];

    type = task->taggedInfo.type;
    switch (type) {

        case K_TASK_THREAD:

            thread = &task->taggedInfo.info.thread;

            if (thread->link.linkEdf.node.delta == 0)
                delta = thread->param.paramEdf.virtualDeadline;
            else 
                delta = thread->link.linkEdf.node.delta;

            deltaChainInsert(&sched->chain, &thread->link.linkEdf.node, delta);
            break;

        case K_TASK_IDLE:
            idle = &task->taggedInfo.info.idle;
            idle->link.linkEdf.idleWindowEpoch = 0;
            break;
        
        default:
            break;
    }
}

kSchedTask_t *kSchedTaskPopEdf(void)
{
    uint32_t cpuId = 0;
    kSchedulerEdf_t *sched = NULL;
    deltaNode_t *node = NULL;

    cpuId = kThisCpuId();
    sched = &gSchedulersEdf[cpuId];

    kSchedFixupChainEdf();

    node = deltaChainPop(&sched->chain);
    return node ? kSchedTaskFromThreadDeltaNodeEdf(node) : &sched->idle;
}

void kSchedTaskTickCallbackEdf(kSchedTask_t *task)
{
    kSchedTaskType_t type = K_TASK_INVALID;

    kSchedThread_t *thread = NULL;
    kSchedLsr_t *lsr = NULL;
    kSchedIdle_t *idle = NULL;

    uint32_t curBudget = 0;
    uint32_t period = 0;

    uint32_t largestEpoch = 0;
    uint32_t epoch = 0;

    type = task->taggedInfo.type;

    switch (type) {

        case K_TASK_THREAD:

            thread = &task->taggedInfo.info.thread;
            
            curBudget = thread->tick.currentBudget;

            if (curBudget == 0) {

                period = thread->tick.period;

#if CONFIG_KSCHED_ALGORITHM_DS
                period = period == 0 ? period : kTickThrottleTimeLeft(&thread->tick.replenishNode);
#endif
            
                if (period >= thread->link.linkEdf.node.delta)
                    thread->link.linkEdf.node.delta = 0;
                else 
                    thread->link.linkEdf.node.delta -= period;

            } else {

                thread->link.linkEdf.node.delta--;

                if (thread->link.linkEdf.node.delta == 0)
                    thread->link.linkEdf.node.delta = thread->param.paramEdf.virtualDeadline;
            }

            break;

        case K_TASK_LSR:
            lsr = &task->taggedInfo.info.lsr;
            largestEpoch = lsr->link.linkEdf.largestWindowEpoch;
            epoch = lsr->link.linkEdf.idleWindowEpoch;

            if (epoch == UINT32_MAX)
                epoch = 0;
            else
                epoch++;

            if (epoch > largestEpoch)
                lsr->link.linkEdf.largestWindowEpoch = epoch;

            lsr->link.linkEdf.idleWindowEpoch = epoch;
            break;

        case K_TASK_IDLE:
            idle = &task->taggedInfo.info.idle;
            largestEpoch = idle->link.linkEdf.largestWindowEpoch;
            epoch = idle->link.linkEdf.idleWindowEpoch;

            if (epoch == UINT32_MAX)
                epoch = 0;
            else
                epoch++;

            if (epoch > largestEpoch)
                idle->link.linkEdf.largestWindowEpoch = epoch;

            idle->link.linkEdf.idleWindowEpoch = epoch;
            break;

        default:
            break;
    }
}

bool kSchedShouldRescheduleEdf(void)
{
    kSchedTask_t *runningTask = NULL;
    uint32_t cpuId = 0;
    kSchedulerEdf_t *sched = NULL;
    kSchedTaskType_t type = K_TASK_INVALID;

    kSchedThread_t *thread = NULL;
    uint32_t delta = 0;
    deltaNode_t *next = NULL;

    bool ret = false;

    runningTask = kTickGetRunningTask();

    K_DYNAMIC_ASSERT(runningTask);
    K_DYNAMIC_ASSERT(runningTask->state == K_TASK_STATE_RUNNING);

    cpuId = runningTask->cpuId;
    sched = &gSchedulersEdf[cpuId];
    type = runningTask->taggedInfo.type;

    switch (type) {

        case K_TASK_THREAD:

            thread = &runningTask->taggedInfo.info.thread;
            delta = thread->link.linkEdf.node.delta;
            kSchedFixupChainEdf();
            next = deltaChainPeek(&sched->chain);
            
            if (next && next->delta < delta)
                ret = true;

            break;

        case K_TASK_IDLE:
            ret = !deltaChainIsEmpty(&sched->chain);
            break;

        default:
            break;
    }

    return ret;
}

void kSchedTaskLsrOutCallbackEdf(kSchedTask_t *task)
{
    kSchedLsr_t *lsr = NULL;
    
    if (task->taggedInfo.type != K_TASK_LSR)
        return;

    lsr = &task->taggedInfo.info.lsr;
    lsr->link.linkEdf.idleWindowEpoch = 0;
}

#endif