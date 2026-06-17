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

#include <workhorse/kSched/kSchedPolicy/kSchedRr.h>

#if CONFIG_KSCHED_POLICY_RR

#include <generated/autoconf.h>
#include <workhorse/kSched/kSchedPolicy/kSchedRrDefs.h>
#include <workhorse/kTick/kTick.h>
#include <export/kCpuInterface.h>
#include <export/kDbgInterface.h>

static 
kSchedulerRr_t gSchedulersRr[CONFIG_KMAX_CPUS];

static 
kSchedOps_t ops = {
    .kSchedInitFn = kSchedInitRr,
    .kSchedTickTransitionCallbackFn = kSchedTickTransitionCallbackRr,
    .kSchedPushFn = kSchedTaskPushRr,
    .kSchedTaskPopFn = kSchedTaskPopRr,
    .kSchedTaskTickCallbackFn = kSchedTaskTickCallbackRr,
    .kSchedShouldRescheduleFn = kSchedShouldRescheduleRr,
    .kSchedTaskLsrOutCallbackFn = kSchedTaskLsrOutCallbackRr,
    .kSchedGetIdleFn = kSchedGetIdleRr
};

static 
inline 
kSchedTask_t *kSchedTaskFromThreadSqNodeRr(sqListNode_t *sqNode)
{
    kSchedThreadLinkRr_t *linkRrPtr = NULL;
    kSchedThreadLink_t *linkPtr = NULL;
    kSchedThread_t *threadPtr = NULL;
    kSchedTask_t *taskPtr = NULL;

    linkRrPtr = containerOf(sqNode, kSchedThreadLinkRr_t, node);
    linkPtr = containerOf(linkRrPtr, kSchedThreadLink_t, linkRr);
    threadPtr = containerOf(linkPtr, kSchedThread_t, link);
    taskPtr = kSchedTaskFromThread(threadPtr);

    return taskPtr;
}

void kSchedOpsInitRr(void)
{
    kSchedOpsInit(&ops);
}

void kSchedInitRr(void)
{
    uint32_t cpuId = 0;
    kSchedulerRr_t *sched = NULL;

    cpuId = kThisCpuId();
    sched = &gSchedulersRr[cpuId];

    kSchedTaskIdleInit(&sched->idle);
}

void kSchedTickTransitionCallbackRr(void)
{

}

void kSchedTaskPushRr(kSchedTask_t *task)
{
    uint32_t cpuId = 0;
    kSchedulerRr_t *sched = NULL;
    kSchedTaskType_t type = K_TASK_INVALID;
    kSchedThread_t *thread = NULL;
    kSchedIdle_t *idle = NULL;
    uint8_t prio = 0;

    cpuId = task->cpuId;
    sched = &gSchedulersRr[cpuId];

    type = task->taggedInfo.type;
    switch (type) {
   
        case K_TASK_THREAD:
            thread = &task->taggedInfo.info.thread;
            prio = thread->param.paramRr.prio;

            sqPushBack(&sched->qs[prio], &thread->link.linkRr.node);
            BITMAP_SET(sched->activeQs, prio); 
            break;

        case K_TASK_IDLE:
            idle = &task->taggedInfo.info.idle;
            idle->link.linkRr.idleWindowEpoch = 0;
            break;

        default:
            break;
    }
}

kSchedTask_t *kSchedTaskPopRr(void)
{
    uint32_t cpuId = 0;
    kSchedulerRr_t *sched = NULL;
    int32_t idx = 0;

    sqListNode_t *sqNode = NULL;
    kSchedTask_t *task = NULL;
    kSchedThread_t *thread = NULL;

    cpuId = kThisCpuId();
    sched = &gSchedulersRr[cpuId];
    idx = bitmapFls(sched->activeQs, CONFIG_KSCHED_POLICY_RR_NUM_PRIO, 0);

    if (idx >= 0) {

        sqNode = sqPopFront(&sched->qs[idx]);
        if (sqIsEmpty(&sched->qs[idx]))
            BITMAP_UNSET(sched->activeQs, idx);

        task = kSchedTaskFromThreadSqNodeRr(sqNode);
        thread = &task->taggedInfo.info.thread;
        thread->link.linkRr.remainingTimesliceTicks = thread->param.paramRr.timesliceTicks;

    } else {
        task = &sched->idle;
    }

    return task;
}

void kSchedTaskTickCallbackRr(kSchedTask_t *task)
{
    kSchedTaskType_t type = K_TASK_INVALID;
    kSchedThread_t *thread = NULL;
    kSchedLsr_t *lsr = NULL;
    kSchedIdle_t *idle = NULL;
    uint32_t largestEpoch = 0;
    uint32_t epoch = 0;

    type = task->taggedInfo.type;
    
    switch (type) {

        case K_TASK_THREAD:
            thread = &task->taggedInfo.info.thread;
            thread->link.linkRr.remainingTimesliceTicks--;
            break;

        case K_TASK_LSR:
            lsr = &task->taggedInfo.info.lsr;
            largestEpoch = lsr->link.linkRr.largestWindowEpoch;
            epoch = lsr->link.linkRr.idleWindowEpoch;

            if (epoch == UINT32_MAX)
                epoch = 0;
            else
                epoch++;

            if (epoch > largestEpoch)
                lsr->link.linkRr.largestWindowEpoch = epoch;

            lsr->link.linkRr.idleWindowEpoch = epoch;
            break;

        case K_TASK_IDLE:
            idle = &task->taggedInfo.info.idle;
            largestEpoch = idle->link.linkRr.largestWindowEpoch;
            epoch = idle->link.linkRr.idleWindowEpoch;

            if (epoch == UINT32_MAX)
                epoch = 0;
            else
                epoch++;

            if (epoch > largestEpoch)
                idle->link.linkRr.largestWindowEpoch = epoch;

            idle->link.linkRr.idleWindowEpoch = epoch;
            break;

        default:
            break;
    }

}

bool kSchedShouldRescheduleRr(void)
{
    kSchedTask_t *runningTask = NULL;
    uint32_t cpuId = 0;
    kSchedulerRr_t *sched = NULL;
    kSchedTaskType_t type = K_TASK_INVALID;
    kSchedThread_t *thread = NULL;
    uint8_t prio = 0;
    int32_t idx = 0;

    bool ret = false;

    runningTask = kTickGetRunningTask();

    K_DYNAMIC_ASSERT(runningTask);
    K_DYNAMIC_ASSERT(runningTask->state == K_TASK_STATE_RUNNING);

    cpuId = runningTask->cpuId;
    sched = &gSchedulersRr[cpuId];
    type = runningTask->taggedInfo.type;

    switch (type) {

        case K_TASK_THREAD:

            thread = &runningTask->taggedInfo.info.thread;
            prio = thread->param.paramRr.prio;
            idx = bitmapFls(sched->activeQs, CONFIG_KSCHED_POLICY_RR_NUM_PRIO, prio);

            if (idx > prio)
                ret = true;
            else if (idx == prio)
                ret = thread->link.linkRr.remainingTimesliceTicks == 0;
            else if (thread->link.linkRr.remainingTimesliceTicks == 0)
                thread->link.linkRr.remainingTimesliceTicks = thread->param.paramRr.timesliceTicks;

            break;

        case K_TASK_IDLE:
            ret = !bitmapIsEmpty(sched->activeQs, CONFIG_KSCHED_POLICY_RR_NUM_PRIO);
            break;

        default:
            break;
    }

    return ret;
}

void kSchedTaskLsrOutCallbackRr(kSchedTask_t *task)
{
    kSchedLsr_t *lsr = NULL;
    
    if (task->taggedInfo.type != K_TASK_LSR)
        return;

    lsr = &task->taggedInfo.info.lsr;
    lsr->link.linkRr.idleWindowEpoch = 0;
}

kSchedTask_t *kSchedGetIdleRr(uint32_t cpuId)
{
    if (!kCpuIdValidate(cpuId))
        return NULL;

    K_DYNAMIC_ASSERT(cpuId < CONFIG_KMAX_CPUS);

    return &gSchedulersRr[cpuId].idle;
}

#endif