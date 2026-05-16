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

#include <plugin/kPlugin.h>
#include <workhorse/kSched/kSched.h>
#include <workhorse/kTick/kTick.h>
#include <export/kCpuInterface.h>
#include <stdWorkhorse.h>

#if CONFIG_KSCHED_POLICY_CYCLIC

static
DEFINE_BITMAPS(frameBmps, CONFIG_KMAX_CPUS, CONFIG_KSCHED_POLICY_CYCLIC_NUM_FRAMES);

#endif

int kPluginInitTaskThread(kSchedTask_t *task, kPluginTaskThreadParam_t *param)
{
    kDomain_t *domain = NULL;
    kSchedThread_t *thread = NULL;
    int err = 0;

    K_DYNAMIC_ASSERT(!gPluginsDone);

#if CONFIG_KSCHED_POLICY_CYCLIC
    uint32_t cpuId = 0;
    uint8_t *bmp = NULL;
    uint32_t low = 0;
    int64_t bit = 0;
#endif

    domain = kDomainUniverseGet(param->domId);
    thread = &task->taggedInfo.info.thread;
    
    if (!domain || !kCpuIdValidate(param->cpuId) || !kSchedTaskCanAdd(param->taskId) || param->budget == 0)
        return -EINVAL;

#if CONFIG_KSCHED_POLICY_RR

    if (param->param.paramRr.prio >= CONFIG_KSCHED_POLICY_RR_NUM_PRIO || param->param.paramRr.timesliceTicks == 0)
        return -EINVAL;

    err = kCpuThreadInfoInit(&thread->archInfo, &param->archParam);
    if (err < 0)
        return err;

#endif

#if CONFIG_KSCHED_POLICY_CYCLIC

    cpuId = param->cpuId;
    bmp = frameBmps[cpuId];

    if (param->param.paramCyclic.timesliceTicks == 0)
        return -EINVAL;

    if (bitmapFfs(param->param.paramCyclic.frameBmp, CONFIG_KSCHED_POLICY_CYCLIC_NUM_FRAMES, 0) < 0)
        return -EINVAL;

    while ((bit = bitmapFfs(param->param.paramCyclic.frameBmp, CONFIG_KSCHED_POLICY_CYCLIC_NUM_FRAMES, low)) != -1) {
        
        if (BITMAP_TEST(bmp, bit))
            return -EINVAL;

        low = bit + 1;
    }

    err = kCpuThreadInfoInit(&thread->archInfo, &param->archParam);
    if (err < 0)
        return err;

    low = 0;
   
    while ((bit = bitmapFfs(param->param.paramCyclic.frameBmp, CONFIG_KSCHED_POLICY_CYCLIC_NUM_FRAMES, low)) != -1) {
        BITMAP_SET(bmp, bit);
        low = bit + 1;
    }

#endif

#if CONFIG_KSCHED_POLICY_EDF

    if (param->param.paramEdf.virtualDeadline == 0)
        return -EINVAL;

    err = kCpuThreadInfoInit(&thread->archInfo, &param->archParam);
    if (err < 0)
        return err;

#endif

    memset(task, 0, sizeof(*task));

    task->cpuId = param->cpuId;

    task->state = K_TASK_STATE_READY;
    task->taggedInfo.type = K_TASK_THREAD;

    thread->param = param->param;
    thread->tick.budget = param->budget;
    thread->tick.currentBudget = param->budget;
    thread->tick.period = param->period;
    
    task->callbacks = param->callbacks;
    task->domain.curDomain = domain;

    kSchedTaskAdd(param->taskId, task);

    kCpuTaskCtxInit(task, domain->invocationInfo._start);
    kTickPluginTaskThreadInit(task);

    return 0;
}

int kPluginInitTaskLsr(kSchedTask_t *task, kPluginTaskLsrParam_t *param)
{
    kSchedLsr_t *lsr = NULL;
    kDomain_t *domain = NULL;
    int err = 0;
    
    K_DYNAMIC_ASSERT(!gPluginsDone);

    lsr = &task->taggedInfo.info.lsr;

    domain = kDomainUniverseGet(param->domId);
    if (!domain || !kCpuIdValidate(param->cpuId) || !kSchedTaskCanAdd(param->taskId))
        return -EINVAL; 

    err = kCpuLsrInfoInit(&lsr->archInfo, &param->archParam);
    if (err < 0)
        return err;

    task->cpuId = param->cpuId;

    task->state = K_TASK_STATE_LSR_DORMANT;
    task->taggedInfo.type = K_TASK_LSR;

    task->callbacks = param->callbacks;
    task->domain.curDomain = domain;
    
    kSchedTaskAdd(param->taskId, task);

    kCpuTaskCtxInit(task, domain->invocationInfo._start);
    kCpuTaskLsrPush(task);

    return 0;
}

int kPluginInitDomain(kDomain_t *domain, kPluginDomainParam_t *param)
{   
    int err = 0;

    K_DYNAMIC_ASSERT(!gPluginsDone);

    if (!kDomainUniverseCanAdd(param->domId))
        return -EINVAL;

    err = kCpuDomainInfoInit(&domain->archInfo, &param->param.archParam);
    if (err < 0)
        return err;

    domain->invocationInfo = param->param.invocationInfo;

    kDomainUniverseAdd(param->domId, domain);
    return 0;
}