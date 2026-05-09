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

#ifndef _K_CPU_INTERFACE_H_
#define _K_CPU_INTERFACE_H_

#include <compiler.h>
#include <workhorse/kSched/kSchedTask.h>

typedef void (*kCpuInvokeRoutineFn_t)(void);

typedef uint32_t (*kThisCpuIdFn_t)(void);
typedef void (*kCpuInvokeAllRendezvousFn_t)(kCpuInvokeRoutineFn_t fn);
typedef void (*kCpuSelfIpiFn_t)(kCpuInvokeRoutineFn_t fn);
typedef void (*kCpuTaskIdleCtxInitFn_t)(kSchedTask_t *task);
typedef void (*kCpuTaskCtxInitFn_t)(kSchedTask_t *task, uintptr_t pc);
typedef void (*kCpuTaskSaveCtxFn_t)(kSchedTask_t *task);
typedef void (*kCpuTaskRestoreCtxFn_t)(kSchedTask_t *task);
typedef uintptr_t (*kCpuSyscallGetReturnAddressFn_t)(void);
typedef void (*kCpuSyscallSetReturnAddressFn_t)(uintptr_t returnAddress);
typedef void (*kCpuExceptionSetReturnAddressFn_t)(uintptr_t returnAddress);
typedef void (*kCpuEnterDomainFn_t)(kDomain_t *domain);
typedef void (*kCpuTaskLsrPushFn_t)(kSchedTask_t *task);
typedef uint32_t (*kCpuEventSenderFn_t)(void);
typedef bool (*kCpuIdValidateFn_t)(uint32_t cpuId);
typedef int (*kCpuLsrInfoInitFn_t)(archSchedLsrInfo_t *info, archSchedLsrParam_t *param);
typedef int (*kCpuDomainInfoInitFn_t)(archDomainInfo_t *info, archDomainParam_t *param);

typedef struct kCpuOps
{
    kThisCpuIdFn_t kThisCpuIdFn;
    kCpuInvokeAllRendezvousFn_t kCpuInvokeAllRendezvousFn;
    kCpuSelfIpiFn_t kCpuSelfIpiFn;
    kCpuTaskIdleCtxInitFn_t kCpuTaskIdleCtxInitFn;
    kCpuTaskCtxInitFn_t kCpuTaskCtxInitFn;
    kCpuTaskSaveCtxFn_t kCpuTaskSaveCtxFn;
    kCpuTaskRestoreCtxFn_t kCpuTaskRestoreCtxFn;
    kCpuSyscallGetReturnAddressFn_t kCpuSyscallGetReturnAddressFn;
    kCpuSyscallSetReturnAddressFn_t kCpuSyscallSetReturnAddressFn;
    kCpuExceptionSetReturnAddressFn_t kCpuExceptionSetReturnAddressFn;
    kCpuEnterDomainFn_t kCpuEnterDomainFn;
    kCpuTaskLsrPushFn_t kCpuTaskLsrPushFn;
    kCpuEventSenderFn_t kCpuEventSenderFn;
    kCpuIdValidateFn_t kCpuIdValidateFn;
    kCpuLsrInfoInitFn_t kCpuLsrInfoInitFn;
    kCpuDomainInfoInitFn_t kCpuDomainInfoInitFn;
    
} kCpuOps_t;

int kCpuOpsInit(kCpuOps_t *ops);

uint32_t kThisCpuId(void);
void kCpuInvokeAllRendezvous(kCpuInvokeRoutineFn_t fn);
void kCpuSelfIpi(kCpuInvokeRoutineFn_t fn);
void kCpuTaskIdleCtxInit(kSchedTask_t *task);
void kCpuTaskCtxInit(kSchedTask_t *task, uintptr_t pc);
void kCpuTaskSaveCtx(kSchedTask_t *task);
void kCpuTaskRestoreCtx(kSchedTask_t *task);
uintptr_t kCpuSyscallGetReturnAddress(void);
void kCpuSyscallSetReturnAddress(uintptr_t returnAddress);
void kCpuExceptionSetReturnAddress(uintptr_t returnAddress);
void kCpuEnterDomain(kDomain_t *domain);
void kCpuTaskLsrPush(kSchedTask_t *task);
uint32_t kCpuEventSender(void);
bool kCpuIdValidate(uint32_t cpuId);
int kCpuLsrInfoInit(archSchedLsrInfo_t *info, archSchedLsrParam_t *param);
int kCpuDomainInfoInit(archDomainInfo_t *info, archDomainParam_t *param);

#endif