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

#ifndef _K_SCHED_EDF_DEFS_H_
#define _K_SCHED_EDF_DEFS_H_

#include <lib/dsa/deltaChain.h>

#if CONFIG_KSCHED_POLICY_EDF

typedef struct kSchedParamEdf
{
    uint32_t virtualDeadline;
} kSchedParamEdf_t;

typedef struct kSchedThreadLinkEdf
{
    deltaNode_t node;
} kSchedThreadLinkEdf_t;

typedef struct kSchedLsrLinkEdf
{
    uint32_t idleWindowEpoch;
    uint32_t largestWindowEpoch;
} kSchedLsrLinkEdf_t;

typedef struct kSchedIdleLinkEdf
{
    uint32_t idleWindowEpoch;
    uint32_t largestWindowEpoch;
} kSchedIdleLinkEdf_t;

typedef struct kSchedParam
{
    kSchedParamEdf_t paramEdf;
} kSchedParam_t;

typedef struct kSchedThreadLink
{
    kSchedThreadLinkEdf_t linkEdf;
} kSchedThreadLink_t;

typedef struct kSchedLsrLink
{
    kSchedLsrLinkEdf_t linkEdf;
} kSchedLsrLink_t;

typedef struct kSchedIdleLink
{
    kSchedIdleLinkEdf_t linkEdf;
} kSchedIdleLink_t;

#endif

#endif