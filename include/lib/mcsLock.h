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

#ifndef _MCS_LOCK_H_
#define _MCS_LOCK_H_

#include <compiler.h>
#include <stdatomic.h>
#include <stdWorkhorse.h>

#define MCS_LOCKED      0
#define MCS_UNLOCKED    1
#define MCS_PAUSED      2

#define INITIALIZE_MCSNODE()    {0}
#define INITIALIZE_MCSLOCK()    {0}

typedef struct mcsNode
{
    atomic_uintptr_t next;
    atomic_uint_fast32_t state;
} mcsNode_t;

typedef struct mcsLock
{
    atomic_uintptr_t tail;
} mcsLock_t;

inline 
void __mcsNodeInit(mcsNode_t *node)
{
    memset(node, 0, sizeof(*node));
}

inline
void __mcsAcquire(mcsLock_t *lock, mcsNode_t *node)
{
    mcsNode_t *prev = NULL;

    prev = (mcsNode_t *)atomic_exchange(&lock->tail, (uintptr_t)node);

    if (prev) {
        atomic_store(&prev->next, (uintptr_t)node);
        spinUntil(atomic_load(&node->state) == MCS_UNLOCKED);
    }
}

inline
void __mcsRelease(mcsLock_t *lock, mcsNode_t *node)
{
    uintptr_t next = 0;
    uintptr_t expected = 0;
    mcsNode_t *nextPtr = NULL;

    next = atomic_load(&node->next);

    if (!next) {

        expected = (uintptr_t)node;
        if (atomic_compare_exchange_strong(&lock->tail, &expected, (uintptr_t)NULL))
            return;

        spinUntil((next = atomic_load(&node->next)) != 0);
    }

    nextPtr = (mcsNode_t *)next;
    atomic_store(&nextPtr->state, MCS_UNLOCKED);
}

inline
bool __mcsTrylock(mcsLock_t *lock, mcsNode_t *node)
{
    uintptr_t expected = 0;
    
    return atomic_compare_exchange_strong(&lock->tail, &expected, (uintptr_t)node);
}

#endif