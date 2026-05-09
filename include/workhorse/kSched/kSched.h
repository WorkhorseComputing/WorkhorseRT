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

#ifndef _K_SCHED_H_
#define _K_SCHED_H_

#include <workhorse/kSched/kSchedTask.h>
#include <export/kDbgInterface.h>
#include <export/kCpuInterface.h>

#define K_SCHED_IDLE_TASK_ID() (kThisCpuId())

typedef void (*kSchedInitFn_t)(void);
typedef void (*kSchedTickTransitionCallbackFn_t)(void);
typedef void (*kSchedTaskPushFn_t)(kSchedTask_t *task);
typedef kSchedTask_t *(*kSchedTaskPopFn_t)(void);
typedef void (*kSchedTaskTickCallbackFn_t)(kSchedTask_t *task);
typedef bool (*kSchedShouldRescheduleFn_t)(void);
typedef void (*kSchedTaskLsrOutCallbackFn_t)(kSchedTask_t *task);

typedef struct kSchedOps
{
    kSchedInitFn_t kSchedInitFn;
    kSchedTickTransitionCallbackFn_t kSchedTickTransitionCallbackFn;
    kSchedTaskPushFn_t kSchedPushFn;
    kSchedTaskPopFn_t kSchedTaskPopFn;
    kSchedTaskTickCallbackFn_t kSchedTaskTickCallbackFn;
    kSchedShouldRescheduleFn_t kSchedShouldRescheduleFn;
    kSchedTaskLsrOutCallbackFn_t kSchedTaskLsrOutCallbackFn;
} kSchedOps_t;

int kSchedTaskAdd(uint32_t taskId, kSchedTask_t *task);
kSchedTask_t *kSchedTaskGet(uint32_t taskId);
bool kSchedTaskCanAdd(uint32_t taskId);

int kSchedOpsInit(kSchedOps_t *ops);

void kSchedInit(void);
void kSchedTickTransitionCallback(void);
void kSchedTaskPush(kSchedTask_t *task);
kSchedTask_t *kSchedTaskPop(void);
void kSchedTaskTickCallback(kSchedTask_t *task);
bool kSchedShouldReschedule(void);
void kSchedTaskLsrOutCallback(kSchedTask_t *task);

inline
void kSchedTaskIdleInit(kSchedTask_t *idle)
{
    uint32_t cpuId = 0;

    cpuId = kThisCpuId();

    idle->cpuId = cpuId;
    idle->taggedInfo.type = K_TASK_IDLE;
    idle->state = K_TASK_STATE_READY;
    
    kCpuTaskIdleCtxInit(idle);

#if CONFIG_KDYNAMIC_ASSERT
    K_DYNAMIC_ASSERT(kSchedTaskAdd(K_SCHED_IDLE_TASK_ID(), idle) == 0);
#else 
    kSchedTaskAdd(cpuId, idle);
#endif
}

#endif