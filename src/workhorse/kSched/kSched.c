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

#include <workhorse/kSched/kSched.h>
#include <export/kDbgInterface.h>
#include <errno.h>

static 
kSchedOps_t *gOps = NULL;

static
bool gOpsInitialized = false;

static 
kSchedTask_t *kTasks[CONFIG_KMAX_TASKS + CONFIG_KMAX_CPUS];

int kSchedTaskAdd(uint32_t taskId, kSchedTask_t *task)
{
    if (taskId >= ARRAY_LEN(kTasks) || kTasks[taskId])
        return -EINVAL;

    kTasks[taskId] = task;
    task->taskId = taskId;
    return 0;
}

kSchedTask_t *kSchedTaskGet(uint32_t taskId)
{
    if (taskId >= ARRAY_LEN(kTasks) || !kTasks[taskId])
        return NULL;

    return kTasks[taskId];
}

bool kSchedTaskCanAdd(uint32_t taskId)
{
    return taskId < ARRAY_LEN(kTasks) && !kTasks[taskId];
}

int kSchedOpsInit(kSchedOps_t *ops)
{
    if (gOpsInitialized)
        return -EINVAL;

    gOps = ops;
    gOpsInitialized = true;
    return 0;
}

void kSchedInit(void)
{
    K_DYNAMIC_ASSERT(gOpsInitialized);
    gOps->kSchedInitFn();
}

void kSchedTickTransitionCallback(void)
{
    K_DYNAMIC_ASSERT(gOpsInitialized);
    gOps->kSchedTickTransitionCallbackFn();
}

void kSchedTaskPush(kSchedTask_t *task)
{
    K_DYNAMIC_ASSERT(gOpsInitialized);
    gOps->kSchedPushFn(task);
}

kSchedTask_t *kSchedTaskPop(void)
{
    K_DYNAMIC_ASSERT(gOpsInitialized);
    return gOps->kSchedTaskPopFn();
}

void kSchedTaskTickCallback(kSchedTask_t *task)
{
    K_DYNAMIC_ASSERT(gOpsInitialized);
    gOps->kSchedTaskTickCallbackFn(task);
}

bool kSchedShouldReschedule(void)
{
    K_DYNAMIC_ASSERT(gOpsInitialized);
    return gOps->kSchedShouldRescheduleFn();
}

void kSchedTaskLsrOutCallback(kSchedTask_t *task)
{
    K_DYNAMIC_ASSERT(gOpsInitialized);
    gOps->kSchedTaskLsrOutCallbackFn(task);
}

kSchedTask_t *kSchedGetIdle(uint32_t cpuId)
{
    K_DYNAMIC_ASSERT(gOpsInitialized);
    return gOps->kSchedGetIdleFn(cpuId);   
}