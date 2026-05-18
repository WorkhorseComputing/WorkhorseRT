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

#define ASM_FILE 1

/* Includes */

#include <mips32Asm.h>
#include <mips32.h>

    .file "mips32MaltaBoot.S"

/* Externs */

.extern __bssStart
.extern __bssEnd
.extern kStart
.extern mips32MaltaCpuApStart

/* Definitions */

.globl _start
.type _start, @function

/* Boot Entry */

.section .boot.start

.align 32
_start:

/* Interrupt entry */

.section .boot.interrupt

.align 32
_startInterrupt:

/* Boot routine */

.section .boot.setup

.align 32
mips32MaltaBootSetup:


/* Boot information */

.section .bss

.align 32
mips32MaltaBootA0:                  .long
mips32MaltaBootA1:                  .long
mips32MaltaBootA2:                  .long
mips32MaltaBootA3:                  .long
mips32MipsMaltaBspEntered:          .long
mips32MaltaApSp:                    .long
mips32MaltaApWatchlo:               .long
mips32MaltaApWatchHi:               .long

.align 32
mips32MaltaBootStackPadding:        .skip 16
mips32MaltaBootStackBottom:
    .skip                           CONFIG_MIPS32_MALTA_KSTACK_SIZE
mips32MaltaBootStackTop: