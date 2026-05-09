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

#include <workhorse/kTick/kTick.h>
#include <workhorse/kSched/kSched.h>
#include <export/kCpuInterface.h>
#include <export/kDbgInterface.h>

bool gPluginsDone = false;

static
kTickMachine_t gTickMachines[CONFIG_KMAX_CPUS];

static 
inline 
kTickMachine_t *kTickGetMachine(void)
{
    uint32_t cpuId = 0;
    kTickMachine_t *machine = NULL;

    cpuId = kThisCpuId();
    machine = &gTickMachines[cpuId];

    return machine;
}

static
inline
kSchedTask_t *kTickSchedTaskPop(void)
{
    kSchedTask_t *newTask = NULL;

    newTask = kSchedTaskPop();

    K_DYNAMIC_ASSERT(newTask);
    K_DYNAMIC_ASSERT(newTask->state == K_TASK_STATE_READY);
    K_DYNAMIC_ASSERT(newTask->taggedInfo.type != K_TASK_LSR);

    return newTask;
}

static
inline 
kSchedTask_t *kTaskFromReplenishNode(deltaNode_t *deltaNode)
{
    kSchedTick_t *tickPtr = NULL;
    kSchedThread_t *threadPtr = NULL;
    kSchedTaskInfo_t *infoPtr = NULL;
    kSchedTaskTaggedInfo_t *taggedInfoPtr = NULL;
    kSchedTask_t *taskPtr = NULL;

    tickPtr = containerOf(deltaNode, kSchedTick_t, replenishNode);
    threadPtr = containerOf(tickPtr, kSchedThread_t, tick);

    infoPtr = containerOf(threadPtr, kSchedTaskInfo_t, thread);
    taggedInfoPtr = containerOf(infoPtr, kSchedTaskTaggedInfo_t, info);
    taskPtr = containerOf(taggedInfoPtr, kSchedTask_t, taggedInfo);

    return taskPtr;
}

static 
inline 
kSchedTask_t *kTaskFromStackqDeferredTickNode(stackqNode_t *nodePtr)
{
    kSchedTask_t *taskPtr = NULL;

    taskPtr = containerOf(nodePtr, kSchedTask_t, deferredTickNode);
    return taskPtr;
}

static
inline
void kTickTaskLoadNextState(kSchedTask_t *task, kSchedState_t nextState)
{
    uint32_t cpuId = 0;
    kTickMachine_t *machine = NULL;

    kSchedState_t oldState = K_TASK_STATE_INVALID;
    kSchedTaskInCallbackFn_t inCallbackFn = NULL;
    kSchedTaskOutCallbackFn_t outCallbackFn = NULL;

#if CONFIG_KSCHED_ALGORITHM_BCS
    kSchedThread_t *thread = NULL;
#endif 

    kSchedLsr_t *lsr = NULL;
    
    cpuId = kThisCpuId();
    machine = &gTickMachines[cpuId];

    oldState = task->state;
    inCallbackFn = task->callbacks.inCallbackFn;
    outCallbackFn = task->callbacks.outCallbackFn;

    K_DYNAMIC_ASSERT(task->taggedInfo.type != K_TASK_INVALID);
    K_DYNAMIC_ASSERT(nextState != K_TASK_STATE_INVALID);

    if (oldState == nextState)
        return;

    task->state = nextState;
    switch (nextState) {

        case K_TASK_STATE_READY:
            K_DYNAMIC_ASSERT(task->taggedInfo.type != K_TASK_LSR);

            kSchedTaskPush(task);
            break;

        case K_TASK_STATE_RUNNING:
            K_DYNAMIC_ASSERT(task->taggedInfo.type != K_TASK_IDLE || !task->domain.curDomain);

            if (task->taggedInfo.type != K_TASK_IDLE)
                kCpuEnterDomain(task->domain.curDomain);

            kCpuTaskRestoreCtx(task);
            machine->runningTask = task;

            if (inCallbackFn)
                inCallbackFn(task);
                
            break;

        case K_TASK_STATE_THREAD_THROTTLED:
            K_DYNAMIC_ASSERT(task->taggedInfo.type == K_TASK_THREAD);

#if CONFIG_KSCHED_ALGORITHM_BCS
            thread = &task->taggedInfo.info.thread;
            deltaChainInsert(&machine->replenishmentChain, &thread->tick.replenishNode, thread->tick.period);
#endif
            break;

        case K_TASK_STATE_LSR_DORMANT:
            K_DYNAMIC_ASSERT(task->taggedInfo.type == K_TASK_LSR);

            kCpuTaskLsrPush(task);
            break;

        case K_TASK_STATE_THREAD_DEFTICK_YIELD:
        case K_TASK_STATE_THREAD_DEFTICK_THROTTLE:
            K_DYNAMIC_ASSERT(task->taggedInfo.type == K_TASK_THREAD);

            stackqPush(&machine->deferredTicks, &task->deferredTickNode);
            break;

        case K_TASK_STATE_DEFTICK_PENDING:
            K_DYNAMIC_ASSERT(task->taggedInfo.type != K_TASK_LSR);

            machine->pendingTaskLsr = task;
            stackqPush(&machine->deferredTicks, &task->deferredTickNode);
            break;

        case K_TASK_STATE_PENDING:
            K_DYNAMIC_ASSERT(task->taggedInfo.type != K_TASK_LSR);

            machine->pendingTaskLsr = task;
            break;

        case K_TASK_STATE_LSR_PENDING:
            K_DYNAMIC_ASSERT(task->taggedInfo.type == K_TASK_LSR);

            lsr = &task->taggedInfo.info.lsr;
            stackqPush(&machine->pendingLsrChain, &lsr->node);
            break;

        case K_TASK_STATE_FAILURE:                
            break;

        default:
            K_DYNAMIC_ASSERT(false);
            break;
    }

    if (oldState == K_TASK_STATE_RUNNING) {

        if (task->taggedInfo.type == K_TASK_LSR)
            kSchedTaskLsrOutCallback(task);

        if (outCallbackFn)
            outCallbackFn(task);
    }
}

static
inline 
void kTickReplenishmentPeriod(kTickMachine_t *machine)
{
    deltaNode_t *deltaNode = NULL;
    kSchedTask_t *replenishedTask = NULL;
    kSchedThread_t *replenishedThread = NULL;

    deltaChainTick(&machine->replenishmentChain);
    while ((deltaNode = deltaChainPopExpired(&machine->replenishmentChain)) != NULL) {

        replenishedTask = kTaskFromReplenishNode(deltaNode);

        K_DYNAMIC_ASSERT(replenishedTask->taggedInfo.type == K_TASK_THREAD);

        replenishedThread = &replenishedTask->taggedInfo.info.thread;

#if CONFIG_KSCHED_ALGORITHM_BCS
        K_DYNAMIC_ASSERT(replenishedTask->state == K_TASK_STATE_THREAD_THROTTLED);

        replenishedThread->tick.currentBudget = replenishedThread->tick.budget;
        kTickTaskLoadNextState(replenishedTask, K_TASK_STATE_READY);
#endif

#if CONFIG_KSCHED_ALGORITHM_DS
        replenishedThread->tick.currentBudget = replenishedThread->tick.budget;

        deltaChainInsert(&machine->replenishmentChain, deltaNode, replenishedThread->tick.period);
        if (replenishedTask->state == K_TASK_STATE_THREAD_THROTTLED)
            kTickTaskLoadNextState(replenishedTask, K_TASK_STATE_READY);
#endif
    }
}

static
inline 
void kTickHandleDeferredTicks(kTickMachine_t *machine)
{   
    stackqNode_t *defTickNode = NULL;
    kSchedTask_t *defTickTask = NULL;
    kSchedState_t state = K_TASK_STATE_INVALID;

    kSchedThread_t *defTickThread = NULL;
    kSchedState_t nextState = K_TASK_STATE_INVALID;

    bool shouldThrottle = false;

    /** Tick a task if it has been deferred, there are 2 main places ticks need to be deferred:
     * 
     * LSR's - if an LSR preempts a task, the task will need to be put onto the deferred tick queue
     * as if LSR's keep arriving before a tick arrives, and end after the tick has arrived, we can't
     * guarantee that the task has been ticked, so we should let it receive the first tick that arrives
     * while in the LSR.
     * 
     * Yields - a task can keep yielding before the next tick occurs, causing it to never get ticked,
     * we also should handle this case by queuing all tasks that have done a yield operation such that
     * they receive the next tick.
     * 
     * I.e., deferred ticks are required here to put an upper bound on timing jitter caused by interrupt
     * handlers preempting threads, and yield operations.
     * 
     * With DS, as the replenishment queue is ticked after the deferred ticks, the threshold for a 
     * throttle is > 1
    */

    while ((defTickNode = stackqPop(&machine->deferredTicks)) != NULL) {

        defTickTask = kTaskFromStackqDeferredTickNode(defTickNode);
        state = defTickTask->state;

        K_DYNAMIC_ASSERT(defTickTask->taggedInfo.type != K_TASK_LSR);

        if (defTickTask->taggedInfo.type == K_TASK_IDLE) {
            
            K_DYNAMIC_ASSERT(state == K_TASK_STATE_DEFTICK_PENDING);

            kSchedTaskTickCallback(defTickTask);
            kTickTaskLoadNextState(defTickTask, K_TASK_STATE_PENDING);
            continue;
        }

        K_DYNAMIC_ASSERT(state == K_TASK_STATE_THREAD_DEFTICK_YIELD || 
                         state == K_TASK_STATE_THREAD_DEFTICK_THROTTLE || 
                         state == K_TASK_STATE_DEFTICK_PENDING);

        defTickThread = &defTickTask->taggedInfo.info.thread;
        nextState = state == K_TASK_STATE_DEFTICK_PENDING ? K_TASK_STATE_PENDING : K_TASK_STATE_READY;

        K_DYNAMIC_ASSERT(defTickThread->tick.currentBudget > 0);

        if (state == K_TASK_STATE_THREAD_DEFTICK_THROTTLE)
            defTickThread->tick.currentBudget = 0;
        else
            defTickThread->tick.currentBudget--;

        if (defTickThread->tick.currentBudget == 0) {

            shouldThrottle = defTickThread->tick.period > 0;

#if CONFIG_KSCHED_ALGORITHM_DS
            shouldThrottle = shouldThrottle && kTickThrottleTimeLeft(&defTickThread->tick.replenishNode) > 1;
#endif
            if (shouldThrottle) {
                
                if (state == K_TASK_STATE_DEFTICK_PENDING)
                    machine->pendingTaskLsr = NULL;

                nextState = K_TASK_STATE_THREAD_THROTTLED;
                
            } else {
                defTickThread->tick.currentBudget = defTickThread->tick.budget;
            }
        }

        kSchedTaskTickCallback(defTickTask);
        kTickTaskLoadNextState(defTickTask, nextState);
    }
}

static
inline
void kSchedTickRunningTask(kTickMachine_t *machine)
{
    kSchedTask_t *runningTask = NULL;
    kSchedThread_t *runningThread = NULL;
    bool shouldThrottle = false;

    runningTask = machine->runningTask;

    K_DYNAMIC_ASSERT(runningTask->state == K_TASK_STATE_RUNNING);

    if (runningTask->taggedInfo.type == K_TASK_THREAD) {

        runningThread = &runningTask->taggedInfo.info.thread;

        K_DYNAMIC_ASSERT(runningThread->tick.currentBudget > 0);

        runningThread->tick.currentBudget--;
        
        shouldThrottle = runningThread->tick.period > 0;

#if CONFIG_KSCHED_ALGORITHM_DS
        shouldThrottle = shouldThrottle && kTickThrottleTimeLeft(&runningThread->tick.replenishNode) > 1;
#endif 

        if (runningThread->tick.currentBudget == 0 && !shouldThrottle)
            runningThread->tick.currentBudget = runningThread->tick.budget;

    }

    kSchedTaskTickCallback(runningTask);
}

static
inline
void kTickPrepareLeaveRunningState(void)
{
    kSchedTask_t *runningTask = NULL;
    kTickMachine_t *machine = NULL;

    runningTask = kTickGetRunningTask();

    K_DYNAMIC_ASSERT(runningTask);
    K_DYNAMIC_ASSERT(runningTask->state == K_TASK_STATE_RUNNING);

    machine = kTickGetMachine();

    machine->runningTask = NULL;
    kCpuTaskSaveCtx(runningTask);
}

static
inline 
void kTickReschedulingPoint(void)
{
    kSchedTask_t *runningTask = NULL;
    kSchedTaskType_t type = K_TASK_INVALID;
    kSchedThread_t *thread = NULL;
    bool budgetDone = false;
    bool reschedule = false;

    runningTask = kTickGetRunningTask();

    K_DYNAMIC_ASSERT(runningTask);
    K_DYNAMIC_ASSERT(runningTask->state == K_TASK_STATE_RUNNING);

    type = runningTask->taggedInfo.type;

    if (type == K_TASK_THREAD) {
        thread = &runningTask->taggedInfo.info.thread;
        budgetDone = thread->tick.currentBudget == 0;
    }

    reschedule = !budgetDone && type != K_TASK_LSR && kSchedShouldReschedule();

    if (budgetDone || reschedule) {

        kTickPrepareLeaveRunningState();
        kTickTaskLoadNextState(runningTask, reschedule ? K_TASK_STATE_READY : K_TASK_STATE_THREAD_THROTTLED);

        kTickTaskLoadNextState(kTickSchedTaskPop(), K_TASK_STATE_RUNNING);
    } 
}

void kTickTransition(void)
{
    kTickMachine_t *machine = NULL;

    machine = kTickGetMachine();

    kSchedTickTransitionCallback();

#if CONFIG_KSCHED_ALGORITHM_BCS
    kTickReplenishmentPeriod(machine);
#endif

    kTickHandleDeferredTicks(machine);
    
    kSchedTickRunningTask(machine);

#if CONFIG_KSCHED_ALGORITHM_DS
    kTickReplenishmentPeriod(machine);
#endif

    kTickReschedulingPoint();
}

kSchedTask_t *kTickGetRunningTask(void)
{
    return kTickGetMachine()->runningTask;
}

uint32_t kTickThrottleTimeLeft(deltaNode_t *node)
{
    kTickMachine_t *machine = NULL;
    machine = kTickGetMachine();

    return deltaChainTimeUntil(&machine->replenishmentChain, node);
}

void kTickLsrPush(kSchedLsr_t *lsr)
{
    kSchedTask_t *task = NULL;

    task = kSchedTaskFromLsr(lsr);
    kTickTaskLoadNextState(task, K_TASK_STATE_LSR_PENDING);
}

void kTickLsrPushCurrent(void)
{
    kTickMachine_t *machine = NULL;
    kSchedTask_t *runningTask = NULL;

    machine = kTickGetMachine();
    runningTask = machine->runningTask;

    if (!runningTask)
        return;

    K_DYNAMIC_ASSERT(runningTask->state == K_TASK_STATE_RUNNING);

    kTickPrepareLeaveRunningState();

    if (runningTask->taggedInfo.type == K_TASK_LSR)
        kTickTaskLoadNextState(runningTask, K_TASK_STATE_LSR_PENDING);
    else
        kTickTaskLoadNextState(runningTask, K_TASK_STATE_DEFTICK_PENDING); 
}

void kTickReschedule(void)
{
    kTickMachine_t *machine = NULL;
    kSchedTask_t *runningTask = NULL;
    kSchedTask_t *pendingTask = NULL;

    stackqNode_t *pendingLsrNode = NULL;
    kSchedTask_t *pendingLsrTask = NULL;

#if CONFIG_KDYNAMIC_ASSERT
    kSchedThread_t *pendingThread = NULL;
#endif

    machine = kTickGetMachine();
    runningTask = machine->runningTask;
    pendingTask = machine->pendingTaskLsr;

    if (runningTask) {

        K_DYNAMIC_ASSERT(runningTask->state == K_TASK_STATE_RUNNING);

        if (runningTask->taggedInfo.type == K_TASK_LSR)
            return;

        K_DYNAMIC_ASSERT(pendingTask == NULL);

        if (stackqIsEmpty(&machine->pendingLsrChain))
            return;

        kTickPrepareLeaveRunningState();
        kTickTaskLoadNextState(runningTask, K_TASK_STATE_DEFTICK_PENDING);
    }

    pendingLsrNode = stackqPop(&machine->pendingLsrChain);
    if (pendingLsrNode) {

        pendingLsrTask = kSchedTaskFromLsrStackqNode(pendingLsrNode);

        K_DYNAMIC_ASSERT(pendingLsrTask->taggedInfo.type == K_TASK_LSR);

        kTickTaskLoadNextState(pendingLsrTask, K_TASK_STATE_RUNNING);
        return;
    }

    if (!pendingTask) {
        kTickTaskLoadNextState(kTickSchedTaskPop(), K_TASK_STATE_RUNNING);
        return;
    }
    
    K_DYNAMIC_ASSERT(pendingTask->taggedInfo.type != K_TASK_LSR);

#if CONFIG_KDYNAMIC_ASSERT

    if (pendingTask->taggedInfo.type == K_TASK_THREAD) {
        pendingThread = &pendingTask->taggedInfo.info.thread;
        K_DYNAMIC_ASSERT(pendingThread->tick.currentBudget != 0);
    }

#endif

    machine->pendingTaskLsr = NULL;
    if (pendingTask->state == K_TASK_STATE_DEFTICK_PENDING) {
        
#if CONFIG_KDYNAMIC_ASSERT
        K_DYNAMIC_ASSERT(stackqPop(&machine->deferredTicks) == &pendingTask->deferredTickNode);
#else
        stackqPop(&machine->deferredTicks);
#endif
    }

    kTickTaskLoadNextState(pendingTask, K_TASK_STATE_RUNNING);
    kTickReschedulingPoint();
}

void kTickSwitchRunningTask(kSchedState_t newState)
{
    kSchedTask_t *runningTask = NULL;

    runningTask = kTickGetRunningTask();

    kTickPrepareLeaveRunningState();
    kTickTaskLoadNextState(runningTask, newState);

    kTickReschedule();
}

void kTickPluginTaskThreadInit(kSchedTask_t *task)
{
#if CONFIG_KSCHED_ALGORITHM_DS

    uint32_t cpuId = 0;
    kTickMachine_t *machine = NULL;
    kSchedThread_t *thread = NULL;
    uint32_t period = 0;

    K_DYNAMIC_ASSERT(!gPluginsDone);
    K_DYNAMIC_ASSERT(task->taggedInfo.type == K_TASK_THREAD);

    cpuId = task->cpuId;
    machine = &gTickMachines[cpuId];;
    thread = &task->taggedInfo.info.thread;
    period = thread->tick.period;

    if (period > 0)
        deltaChainInsert(&machine->replenishmentChain, &thread->tick.replenishNode, period);

#else 
    K_DYNAMIC_ASSERT(!gPluginsDone);
    K_DYNAMIC_ASSERT(task->taggedInfo.type == K_TASK_THREAD);
#endif

    kSchedTaskPush(task);
}