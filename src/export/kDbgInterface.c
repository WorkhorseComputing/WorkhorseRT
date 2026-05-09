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

#include <export/kDbgInterface.h>
#include <generated/autoconf.h>
#include <lib/nanoprintf.h>
#include <errno.h>

static 
kDbgOps_t *gOps = NULL; 

static 
bool gOpsInitialized = false;

int kDbgOpsInit(kDbgOps_t *ops)
{
    if (gOpsInitialized)
        return -EINVAL;

    gOps = ops;
    gOpsInitialized = true;
    return 0;
}

void kDbgStr(const char *str)
{
    if (gOpsInitialized)
        gOps->kDbgStrFn(str);
}

int kDbgStrf(const char *fmt, ...)
{
    char buf[CONFIG_KBUF_SIZE];

    va_list args;
    va_start(args, fmt);
    int len = npf_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    kDbgStr(buf);

    return len;
}