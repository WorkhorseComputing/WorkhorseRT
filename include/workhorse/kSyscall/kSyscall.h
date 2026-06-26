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

#ifndef _K_SYSCALL_H_
#define _K_SYSCALL_H_

#include <compiler.h>

#define WORKHORSE_SYS_INVOCATION_CTRL           0
#define WORKHORSE_SYS_SCHED_CTRL                1
#define WORKHORSE_SYS_GET_DOM_ID                2
#define WORKHORSE_SYS_GET_TASK_ID               3
#define WORKHORSE_SYS_GET_TASK_TYPE             4
#define WORKHORSE_SYS_GET_CPU_ID                5

#define WORKHORSE_INVOCATION_CTRL_DO_RETURN                 0
#define WORKHORSE_INVOCATION_CTRL_DO_IPC                    1
#define WORKHORSE_INVOCATION_CTRL_GET_INVOCATIONS_AVAIL     2
#define WORKHORSE_INVOCATION_CTRL_GET_INVOKING_DOM_ID       3
#define WORKHORSE_INVOCATION_CTRL_GET_INVOCATION_TYPE       4
#define WORKHORSE_INVOCATION_CTRL_GET_RETURN_ADDRESS        5
#define WORKHORSE_INVOCATION_CTRL_GET_VMEM_FAULT_ADDRESS    6
#define WORKHORSE_INVOCATION_CTRL_GET_ERROR_CODE            7
#define WORKHORSE_INVOCATION_CTRL_SET_RETURN_ADDRESS        8

#define WORKHORSE_INVOCATION_TYPE_IPC               0
#define WORKHORSE_INVOCATION_TYPE_VMEM_FAULT        1
#define WORKHORSE_INVOCATION_TYPE_ILLEGAL_OPCODE    2
#define WORKHORSE_INVOCATION_TYPE_ALIGNMENT         3
#define WORKHORSE_INVOCATION_TYPE_DEBUG             4
#define WORKHORSE_INVOCATION_TYPE_ARITHMETIC        5
#define WORKHORSE_INVOCATION_TYPE_OTHER             6

#define WORKHORSE_SCHED_CTRL_YIELD              0
#define WORKHORSE_SCHED_CTRL_THROTTLE           1
#define WORKHORSE_SCHED_CTRL_LSR_DONE           2
#define WORKHORSE_SCHED_CTRL_FAILURE            3
#define WORKHORSE_SCHED_CTRL_SLEEP_MS           4

#define WORKHORSE_TASK_TYPE_THREAD              0
#define WORKHORSE_TASK_TYPE_LSR                 1
#define WORKHORSE_TASK_TYPE_IDLE                2

intptr_t kSysInvocationCtrl(uintptr_t ctrl, uintptr_t val);
intptr_t kSysSchedCtrl(uintptr_t ctrl, uintptr_t val);
intptr_t kSysGetDomId(void);
intptr_t kSysGetTaskId(void);
intptr_t kSysGetTaskType(void);
intptr_t kSysGetCpuId(void);

#endif