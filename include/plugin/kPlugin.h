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

#ifndef _K_PLUGIN_H_
#define _K_PLUGIN_H_

#include <workhorse/kSched/kSchedTask.h>
#include <export/kDbgInterface.h>
#include <defs.h>
#include <errno.h>

#define K_PLUGIN_REGISTRY ".kPlugin"
#define __kPluginRegistry(order) __attribute__ ((section(K_PLUGIN_REGISTRY "." #order), used))

typedef void (*kPluginFn_t)(void);

typedef struct kPluginCall
{
    const char *name;
    kPluginFn_t fn;
} kPlugin_t;

extern kPlugin_t __kPluginStart[];
extern kPlugin_t __kPluginEnd[];

#define kPluginCount() (((uintptr_t)__kPluginEnd - (uintptr_t)__kPluginStart) / sizeof(kPlugin_t))

#define K_REGISTER_PLUGIN(_name, _fn, order)        \
    SIZE_ASSERT(#order, 4);                         \
    __kPluginRegistry(order)                        \
    static const kPlugin_t kPlugin_##_name = {      \
        .name = (#_name),                           \
        .fn = (_fn),                                \
    }

typedef struct kPluginTaskThreadParam
{
    uint32_t cpuId;
    uint32_t domId;
    uint32_t taskId;
    uint32_t budget;
    uint32_t period;
    kSchedParam_t param;
    archSchedThreadParam_t archParam;
    kSchedTaskCallbacks_t callbacks;
} kPluginTaskThreadParam_t;

typedef struct kPluginTaskLsrParam
{
    uint32_t cpuId;
    uint32_t domId;
    uint32_t taskId;
    archSchedLsrParam_t param;
    kSchedTaskCallbacks_t callbacks;
} kPluginTaskLsrParam_t;

typedef struct kPluginDomainParam
{
    uint32_t domId;
    kDomainParam_t param;
} kPluginDomainParam_t;

#define kPluginPrintf(fmt, ...) kDbgStrf((fmt), ##__VA_ARGS__)

int kPluginInitTaskThread(kSchedTask_t *task, kPluginTaskThreadParam_t *param);
int kPluginInitTaskLsr(kSchedTask_t *task, kPluginTaskLsrParam_t *param);
int kPluginInitDomain(kDomain_t *domain, kPluginDomainParam_t *param);

#endif