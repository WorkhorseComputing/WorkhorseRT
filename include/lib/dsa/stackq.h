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

#ifndef _STACKQ_H_
#define _STACKQ_H_

#include <compiler.h>

struct stackqNode;
typedef struct stackqNode stackqNode_t;

typedef struct stackqNode
{
    stackqNode_t *under;
} stackqNode_t;

typedef struct
{
    stackqNode_t *top;
} stackq_t;

inline
void stackqPush(stackq_t *q, stackqNode_t *node)
{
    node->under = q->top;
    q->top = node;
}

inline
stackqNode_t *stackqPop(stackq_t *q)
{
    stackqNode_t *top = NULL;

    top = q->top;
    if (top) {
        q->top = top->under;
        top->under = NULL;
    }

    return top;
}

inline 
bool stackqIsEmpty(stackq_t *q)
{
    return !q->top;
}

#endif