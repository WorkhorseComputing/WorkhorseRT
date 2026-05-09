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

#include <ia32eUart.h>
#include <ia32eAsm.h>
#include <export/kDbgInterface.h>
#include <lib/mcsLock.h>

static 
mcsLock_t ia32eUartLock = INITIALIZE_MCSLOCK();

static 
kDbgOps_t ops = {
    .kDbgStrFn = ia32eUartPrint
};

void ia32eUartPrint(const char *str)
{
    mcsNode_t node = {0};
    uint64_t status = 0;

    __mcsNodeInit(&node);
    status = cpuReadStatus();

    cpuDisableInterrupts();

    __mcsAcquire(&ia32eUartLock, &node);
    __ia32eUartTransmitStr(IA32E_UART_COM_PORT, str);
    __mcsRelease(&ia32eUartLock, &node);

    cpuWriteStatus(status);
}

void ia32eUartInit(void)
{
    if (__ia32eUartInit(IA32E_UART_COM_PORT) == 0)
        kDbgOpsInit(&ops);
}