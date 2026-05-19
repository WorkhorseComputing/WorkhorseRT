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

#include <generated/autoconf.h>
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

.globl mips32MaltaBootA0
.type mips32MaltaBootA0, @object

.globl mips32MaltaBootA1
.type mips32MaltaBootA1, @object

.globl mips32MaltaBootA2
.type mips32MaltaBootA2, @object

.globl mips32MaltaBootA3
.type mips32MaltaBootA3, @object

.globl mips32MaltaApSp
.type mips32MaltaApSp, @object

.globl mips32MaltaApWatchLo
.type mips32MaltaApWatchLo, @object

.globl mips32MaltaApWatchHi
.type mips32MaltaApWatchHi, @object

/* Boot Entry */

.section .boot.start

.balign 32
_start:
    b mips32MaltaBootSetup
    nop

/* Interrupt entry */

.section .boot.interrupt

.balign 32
_startInterrupt:
    nop

/* Boot routines */

.section .boot.setup

.balign 32
mips32MaltaBootUartInit:
    nop

.balign 32
mips32MaltaBootUartLog:
    nop

.balign 32
mips32MaltaBootSetup:
    nop

/* Error messages */

.section .data

.balign 32
mips32MaltaBootFpuNotPresent:       .asciz "Boot Error, FPU not supported"
mips32MaltaBootFrNotPresent:        .asciz "Boot error, FR not supported"
mips32MaltaBootDspNotPresent:       .asciz "Boot error, Dsp not supported"
mips32MaltaBootMsaNotPresent:       .asciz "Boot error, Msa not supported"

/* Boot information */

.section .bss

.balign 32
mips32MaltaBootA0:                  .long
mips32MaltaBootA1:                  .long
mips32MaltaBootA2:                  .long
mips32MaltaBootA3:                  .long
mips32MipsMaltaBspEntered:          .long
mips32MaltaApSp:                    .long
mips32MaltaApWatchLo:               .long
mips32MaltaApWatchHi:               .long

.balign 32
mips32MaltaFeatureBitmasks:         .skip (CONFIG_KMAX_CPUS * 4)

.balign 32
mips32MaltaBootStackPadding:        .skip 16
mips32MaltaBootStackBottom:
    .skip                           CONFIG_MIPS32_MALTA_KSTACK_SIZE
mips32MaltaBootStackTop: