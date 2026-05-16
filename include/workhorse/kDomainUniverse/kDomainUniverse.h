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

#ifndef _K_DOMAIN_UNIVERSE_H_
#define _K_DOMAIN_UNIVERSE_H_

#include <generated/autoconf.h>
#include <defs.h>
#include <lib/dsa/bitmap.h>

typedef enum kDomainInvocationType
{
    
    K_INVOCATION_INVALID =                      0,
    K_INVOCATION_IPC =                          1,
    K_INVOCATION_EXCEPTION_VMEM_FAULT =         2,
    K_INVOCATION_EXCEPTION_ILLEGAL_OPCODE =     3,
    K_INVOCATION_EXCEPTION_ALIGNMENT =          4,
    K_INVOCATION_EXCEPTION_DEBUG =              5,
    K_INVOCATION_EXCEPTION_ARITHMETIC =         6,
    K_INVOCATION_EXCEPTION_OTHER =              7
} kDomainInvocationType_t;

typedef struct kDomainEntryPoint
{
    uintptr_t _entry;
    bool valid;
} kDomainEntryPoint_t;

typedef struct kDomainInvocationInfo
{
    DEFINE_BITMAP(invokePermMap, CONFIG_KMAX_DOMAINS);

    uintptr_t _start;
    kDomainEntryPoint_t invocationIpc;
    kDomainEntryPoint_t invocationExceptionVmemFault;
    kDomainEntryPoint_t invocationExceptionIllegalOpcode;
    kDomainEntryPoint_t invocationExceptionAlignment;
    kDomainEntryPoint_t invocationExceptionDebug;
    kDomainEntryPoint_t invocationExceptionArithmetic;
    kDomainEntryPoint_t invocationExceptionOther;
} kDomainInvocationInfo_t;

typedef struct kDomainParam
{
    kDomainInvocationInfo_t invocationInfo;
} kDomainParam_t;

typedef struct kDomain
{
    uint32_t domId;

    archDomainInfo_t archInfo;
    kDomainInvocationInfo_t invocationInfo;
} kDomain_t;

typedef struct kDomainInvocationEntry
{
    kDomain_t *invokingDomain;
    kDomainInvocationType_t type;
    uintptr_t returnAddress;
    uintptr_t vmemFaultAddress;
    uintptr_t errorCode;
} kDomainInvocationEntry_t;

typedef struct kDomainUniverse
{
    kDomain_t *domainTable[CONFIG_KMAX_DOMAINS];
} kDomainUniverse_t;

kDomain_t *kDomainUniverseGet(uint32_t domId);
int kDomainUniverseAdd(uint32_t domId, kDomain_t *domain);
bool kDomainUniverseCanAdd(uint32_t domId);
bool kDomainUniverseAuthenticateInvocation(uint32_t domId, kDomain_t *invokingDomain);

int kDomainPushInvocationEntry(uintptr_t *newPc, kDomain_t *invokedDomain, kDomainInvocationType_t type, 
                               uintptr_t returnAddress, uintptr_t vmemFaultAddress, uintptr_t errorCode);

int kDomainPopInvocationEntry(uintptr_t *newPc, kDomain_t **newDomain);
kDomainInvocationEntry_t *kDomainReadInvocationEntry(void);

#endif