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

#ifndef _DQ_H_
#define _DQ_H_

#include <compiler.h>

struct dqListNode;
typedef struct dqListNode dqListNode_t;

typedef struct dqListNode
{
    dqListNode_t *next;
    dqListNode_t *prev;
} dqListNode_t;

typedef struct dq
{
    dqListNode_t *front;
    dqListNode_t *back;
} dq_t;

inline
void dqInit(dq_t *q)
{
    q->front = NULL;
    q->back = NULL;
}

inline 
void dqPushBack(dq_t *q, dqListNode_t *node)
{
    node->next = NULL;
    node->prev = q->back;

    if (node->prev)
        node->prev->next = node;
    else
        q->front = node;

    q->back = node;
}

inline
void dqPushFront(dq_t *q, dqListNode_t *node)
{
    node->prev = NULL;
    node->next = q->front;
    
    if (node->next)
        node->next->prev = node;
    else
        q->back = node;

    q->front = node;
}

inline 
dqListNode_t *dqPopFront(dq_t *q)
{
    dqListNode_t *node = NULL;

    node = q->front;
    if (!node)
        return NULL;

    q->front = node->next;

    if (node->next) {
        node->next->prev = NULL;
        node->next = NULL;
    } else {
        q->back = NULL;
    }

    return node;
}

inline
dqListNode_t *dqPopBack(dq_t *q)
{
    dqListNode_t *node = NULL;
    
    node = q->back;
    if (!node)
        return NULL;

    q->back = node->prev;

    if (node->prev) {
        node->prev->next = NULL;
        node->prev = NULL;
    } else {
        q->front = NULL;
    }

    return node;
}

inline
void dqInsertPrev(dq_t *q, dqListNode_t *prev, dqListNode_t *node)
{
    dqListNode_t *next = NULL;

    if (!prev) {
        dqPushFront(q, node);
        return;
    }

    next = prev->next;
    if (!next) {
        dqPushBack(q, node);
        return;
    }

    prev->next = node;
    next->prev = node;

    node->prev = prev;
    node->next = next;
}

inline
void dqInsertNext(dq_t *q, dqListNode_t *next, dqListNode_t *node)
{
    dqListNode_t *prev = NULL;
    
    if (!next) {
        dqPushBack(q, node);
        return;
    }

    prev = next->prev;
    if (!prev) {
        dqPushFront(q, node);
        return;
    }

    prev->next = node;
    next->prev = node;

    node->prev = prev;
    node->next = next;
}

inline
void dqDequeue(dq_t *q, dqListNode_t *node)
{
    dqListNode_t *prev = NULL;
    dqListNode_t *next = NULL;

    prev = node->prev;
    next = node->next;

    if (prev)
        prev->next = next;
    else 
        q->front = next;

    if (next)
        next->prev = prev;
    else
        q->back = prev;

    node->prev = NULL;
    node->next = NULL;
}

inline 
bool dqIsEmpty(dq_t *q)
{
    return !q->front;
}

#endif