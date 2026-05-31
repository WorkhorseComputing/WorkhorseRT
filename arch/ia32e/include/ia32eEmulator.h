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

#ifndef _IA32E_EMULATOR_H_
#define _IA32E_EMULATOR_H_

#include <generated/autoconf.h>
#include <ia32eAsm.h>

typedef struct ATTR_PACKED ia32eVmexitRegs
{
    uint64_t cr2;
    uint64_t dr0;
    uint64_t dr1;
    uint64_t dr2;
    uint64_t dr3;
    uint64_t dr6;
    ia32eRegs_t regs;
} ia32eVmexitRegs_t;
STATIC_ASSERT((sizeof(ia32eVmexitRegs_t) % 16) == 0);

typedef enum ia32eEmulatorMode
{
    IA32E_EMULATOR_INVALID =    0,
    IA32E_EMULATOR_16 =         1,
    IA32E_EMULATOR_V8086 =      2,
    IA32E_EMULATOR_32 =         3,
    IA32E_EMULATOR_64 =         4
} ia32eEmulatorMode_t;

typedef void (*ia32eEmulatorFn_t)(ia32eVmexitRegs_t *regs);

#if CONFIG_IA32E_VTX

void ia32eEmulatorVcpuFailureEntry(void);
void ia32eEmulatorDispatcher(ia32eVmexitRegs_t *regs);

#endif

#endif