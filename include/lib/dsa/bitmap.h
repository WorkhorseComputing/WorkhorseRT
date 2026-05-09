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

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <compiler.h>

#define DEFINE_BITMAP(name, bitCnt)     uint8_t (name)[((bitCnt) + 7) / 8]
#define DEFINE_BITMAPS(name, bins, bitCnt)     uint8_t (name)[(bins)][((bitCnt) + 7) / 8]

#define BITMAP_SET(name, bit)           (name[(bit)/8] |= (1 << ((bit) % 8)))
#define BITMAP_UNSET(name, bit)         (name[(bit)/8] &= ~(1 << ((bit) % 8)))
#define BITMAP_TEST(name, bit)          ((name[(bit)/8] & (1 << ((bit) % 8))) != 0)

#define BITMAP_SET_ALL(name, size)      (memset((name), 0xff, (size)))
#define BITMAP_UNSET_ALL(name, size)    (memset((name), 0, (size)))

inline
int64_t bitmapFls(uint8_t *bitmap, uint32_t bitCnt, uint32_t lowBound)
{
    int64_t i = 0;
    uint32_t idx = 0;
    uint32_t shift = 0;

    if (bitCnt == 0)
        return -1;

    for (i = (int64_t)bitCnt - 1; i >= (int64_t)lowBound; i--) {

        idx = i / 8;
        shift = i % 8;

        if ((bitmap[idx] & (1U << shift)) != 0)
            return i;
    }

    return -1;
}

inline
int64_t bitmapFfs(uint8_t *bitmap, uint32_t bitCnt, uint32_t lowBound)
{
    int64_t i = 0;
    uint32_t idx = 0;
    uint32_t shift = 0;

    if (bitCnt == 0)
        return -1;

    for (i = lowBound; i < bitCnt; i++) {

        idx = i / 8;
        shift = i % 8;

        if ((bitmap[idx] & (1U << shift)) != 0)
            return i;
    }

    return -1;
}

inline
bool bitmapIsEmpty(uint8_t *bitmap, uint32_t bitCnt)
{
    uint32_t i = 0;
    size_t size = 0;

    size = (bitCnt + 7) / 8;

    for (i = 0; i < size; i++) {

        if (bitmap[i] != 0)
            return false;
    }

    return true;
}

#endif