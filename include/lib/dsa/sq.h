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

#ifndef _SQ_H_
#define _SQ_H_

#include <compiler.h>

struct sqListNode;
typedef struct sqListNode sqListNode_t;

typedef struct sqListNode
{
    sqListNode_t *next;
} sqListNode_t;

typedef struct sq
{
    sqListNode_t *front;
    sqListNode_t *back;
} sq_t;

inline
void sqInit(sq_t *q)
{
    q->front = NULL;
    q->back = NULL;
}

inline
void sqPushBack(sq_t *q, sqListNode_t *node)
{
    node->next = NULL;

    if (q->back) {
        q->back->next = node;
        q->back = node;
    } else {
        q->back = node;
        q->front = node;
    }
}

inline
void sqPushFront(sq_t *q, sqListNode_t *node)
{
    node->next = q->front;
    q->front = node;
    if (!node->next)
        q->back = node;
}

inline
sqListNode_t *sqPopFront(sq_t *q)
{
    sqListNode_t *front = q->front;
    if (!front)
        return front;

    q->front = front->next;

    if (q->front)
        front->next = NULL;
    else
        q->back = NULL;

    return front;
}

inline
void sqInsert(sq_t *q, sqListNode_t *prev, sqListNode_t *node)
{
    sqListNode_t *next = NULL;

    if (!prev) {
        sqPushFront(q, node);
        return;
    }

    next = prev->next;
    if (!next) {
        sqPushBack(q, node);
        return;
    }

    prev->next = node;
    node->next = next;
}

inline
bool sqIsEmpty(sq_t *q)
{
    return !q->front;
}

#endif