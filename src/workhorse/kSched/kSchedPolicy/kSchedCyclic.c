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

#include <workhorse/kSched/kSchedPolicy/kSchedCyclic.h>

#if CONFIG_KSCHED_POLICY_CYCLIC

#include <workhorse/kSched/kSchedPolicy/kSchedCyclicDefs.h>
#include <workhorse/kTick/kTick.h>

static
kSchedulerCyclic_t gSchedulersCyclic[CONFIG_KMAX_CPUS];

static
kSchedOps_t ops = {
    .kSchedInitFn = kSchedInitCyclic,
    .kSchedTickTransitionCallbackFn = kSchedTickTransitionCallbackCyclic,
    .kSchedPushFn = kSchedTaskPushCyclic,
    .kSchedTaskPopFn = kSchedTaskPopCyclic,
    .kSchedTaskTickCallbackFn = kSchedTaskTickCallbackCyclic,
    .kSchedShouldRescheduleFn = kSchedShouldRescheduleCyclic,
    .kSchedTaskLsrOutCallbackFn = kSchedTaskLsrOutCallbackCyclic
};

void kSchedOpsInitCyclic(void)
{
    kSchedOpsInit(&ops); 
}

void kSchedInitCyclic(void)
{
    uint32_t cpuId = 0;
    kSchedulerCyclic_t *sched = NULL;

    cpuId = kThisCpuId();
    sched = &gSchedulersCyclic[cpuId];

    kSchedTaskIdleInit(&sched->idle);
}

void kSchedTickTransitionCallbackCyclic(void)
{

}

void kSchedTaskPushCyclic(kSchedTask_t *task)
{
    uint32_t cpuId = 0;
    kSchedulerCyclic_t *sched = NULL;
    kSchedTaskType_t type = K_TASK_INVALID;
    kSchedThread_t *thread = NULL;
    kSchedIdle_t *idle = NULL;
    uint8_t *bmp = NULL;
    int64_t bit = 0;
    uint32_t low = 0;

    cpuId = task->cpuId;
    sched = &gSchedulersCyclic[cpuId];
    type = task->taggedInfo.type;

    switch (type) {

        case K_TASK_THREAD:
            thread = &task->taggedInfo.info.thread;
            bmp = thread->param.paramCyclic.frameBmp;

            if (!thread->link.linkCyclic.initialized) {

                while ((bit = bitmapFfs(bmp, CONFIG_KSCHED_POLICY_CYCLIC_NUM_FRAMES, low)) != -1) {
                    sched->frames[bit] = task;
                    low = bit + 1;
                }

                thread->link.linkCyclic.initialized = true;
            }

            sched->numReadyThreads++;
            break;

        case K_TASK_IDLE:
            idle = &task->taggedInfo.info.idle;
            idle->link.linkCyclic.idleWindowEpoch = 0;
            break;

        default:
            break;
    }
}

kSchedTask_t *kSchedTaskPopCyclic(void)
{
    uint32_t cpuId = 0;
    kSchedulerCyclic_t *sched = NULL;
    uint32_t clock = 0;
    uint32_t i = 0;
    uint32_t frame = 0;

    kSchedTask_t *task = NULL;
    kSchedThread_t *thread = NULL;

    cpuId = kThisCpuId();
    sched = &gSchedulersCyclic[cpuId];
    clock = sched->clock;

    if (sched->numReadyThreads == 0)
        return &sched->idle;

    for (i = 0; i < ARRAY_LEN(sched->frames); i++) {

        frame = (clock + i) % ARRAY_LEN(sched->frames);

        if (sched->frames[frame] && sched->frames[frame]->state == K_TASK_STATE_READY) {
            task = sched->frames[frame];
            thread = &task->taggedInfo.info.thread;

            thread->link.linkCyclic.remainingTimesliceTicks = thread->param.paramCyclic.timesliceTicks;
            break;
        }
    }

    K_DYNAMIC_ASSERT(task);

    sched->clock = frame + 1;
    sched->numReadyThreads--;
    return task;
}

void kSchedTaskTickCallbackCyclic(kSchedTask_t *task)
{
    kSchedTaskType_t type = K_TASK_INVALID;
    kSchedThread_t *thread = NULL;
    kSchedIdle_t *idle = NULL;
    kSchedLsr_t *lsr = NULL;
    uint32_t largestEpoch = 0;
    uint32_t epoch = 0;

    type = task->taggedInfo.type;

    switch (type) {

        case K_TASK_THREAD:
            thread = &task->taggedInfo.info.thread;
            thread->link.linkCyclic.remainingTimesliceTicks--;
            break;

        case K_TASK_LSR:
            lsr = &task->taggedInfo.info.lsr;
            largestEpoch = lsr->link.linkCyclic.largestWindowEpoch;
            epoch = lsr->link.linkCyclic.idleWindowEpoch;

            if (epoch == UINT32_MAX)
                epoch = 0;
            else
                epoch++;

            if (epoch > largestEpoch)
                lsr->link.linkCyclic.largestWindowEpoch = epoch;

            lsr->link.linkCyclic.idleWindowEpoch = epoch;
            break;

        case K_TASK_IDLE:
            idle = &task->taggedInfo.info.idle;
            largestEpoch = idle->link.linkCyclic.largestWindowEpoch;
            epoch = idle->link.linkCyclic.idleWindowEpoch;

            if (epoch == UINT32_MAX)
                epoch = 0;
            else
                epoch++;

            if (epoch > largestEpoch)
                idle->link.linkCyclic.largestWindowEpoch = epoch;

            idle->link.linkCyclic.idleWindowEpoch = epoch;
            break;

        default:
            break;
    }
}

bool kSchedShouldRescheduleCyclic(void)
{
    uint32_t cpuId = 0;
    kSchedulerCyclic_t *sched = NULL;
    kSchedTask_t *runningTask = NULL;
    kSchedTaskType_t type = K_TASK_INVALID;
    kSchedThread_t *thread = NULL;

    bool ret = false;

    cpuId = kThisCpuId();
    sched = &gSchedulersCyclic[cpuId];
    runningTask = kTickGetRunningTask();
    type = runningTask->taggedInfo.type;

    switch (type) {

        case K_TASK_THREAD:
            thread = &runningTask->taggedInfo.info.thread;
            
            if (thread->link.linkCyclic.remainingTimesliceTicks == 0) {

                if (sched->numReadyThreads == 0)
                    thread->link.linkCyclic.remainingTimesliceTicks = thread->param.paramCyclic.timesliceTicks;
                else
                    ret = true;
            }

            break;

        case K_TASK_IDLE:
            ret = sched->numReadyThreads > 0;
            break;

        default:
            break;
    }

    return ret;
}

void kSchedTaskLsrOutCallbackCyclic(kSchedTask_t *task)
{
    kSchedLsr_t *lsr = NULL;
    
    if (task->taggedInfo.type != K_TASK_LSR)
        return;

    lsr = &task->taggedInfo.info.lsr;
    lsr->link.linkCyclic.idleWindowEpoch = 0;
}

#endif