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

#ifndef _STD_WORKHORSE_H_
#define _STD_WORKHORSE_H_

#include <compiler.h>

#define msToTicks(ms, freqHz) (((ms) * (freqHz)) / 1000ULL)
#define usToTicks(us, freqHz) (((us) * (freqHz)) / 1000000ULL)
#define testBitLe(val, bit) ((((val) >> (bit)) & 1) != 0)

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

#define READ_ONCE(x)					                        \
({      					                                    \
	union {typeof((x)) __val; char __c[1];} __u = {.__c = {0}}; \
                                                                \
	__readOnceSize(&(x), __u.__c, sizeof((x)));	                \
                                                                \
	__u.__val;					                                \
})

#define WRITE_ONCE(x, val)				                                \
({							                                            \
	union {typeof((x)) __val; char __c[1];} __u = {.__val = (val)};     \
                                                                        \
	__writeOnceSize(&(x), __u.__c, sizeof((x)));	                    \
                                                                        \
	__u.__val;					                                        \
})

void *memset(void *str, int c, size_t n);

void *memcpy(void *dest, const void *src, size_t n);

inline 
void __readOnceSize(const volatile void *p, void *res, int size)
{
	switch (size) {
		case 1: *(uint8_t  *)res = *(volatile uint8_t  *)p; break;
		case 2: *(uint16_t *)res = *(volatile uint16_t *)p; break;
		case 4: *(uint32_t *)res = *(volatile uint32_t *)p; break;
		case 8: *(uint64_t *)res = *(volatile uint64_t *)p; break;
		default:
			barrier();
			memcpy((void *)res, (const void *)p, size);
			barrier();
        	break;
	    }
}

inline 
void __writeOnceSize(volatile void *p, void *res, int size)
{
	switch (size) {
		case 1: *(volatile  uint8_t *)p = *(uint8_t  *)res; break;
		case 2: *(volatile uint16_t *)p = *(uint16_t *)res; break;
		case 4: *(volatile uint32_t *)p = *(uint32_t *)res; break;
		case 8: *(volatile uint64_t *)p = *(uint64_t *)res; break;
		default:
			barrier();
			memcpy((void *)p, (const void *)res, size);
			barrier();
        	break;
	}
}

inline 
int fls32(uint32_t val)
{
    if (val == 0)
        return -1;

    int bit = 31;

    if (!(val & 0xffff0000)) {
        val <<= 16;
        bit -= 16;
    }

    if (!(val & 0xff000000)) {
        val <<= 8;
        bit -= 8;
    }

    if (!(val & 0xf0000000)) {
        val <<= 4;
        bit -= 4;
    }

    if (!(val & 0xc0000000)) {
        val <<= 2;
        bit -= 2;
    }

    if (!(val & 0x80000000))
        bit -= 1;

    return bit;
}

#endif