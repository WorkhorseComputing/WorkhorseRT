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

#ifndef _DELTA_CHAIN_H_
#define _DELTA_CHAIN_H_

#include <lib/dsa/dq.h>

typedef struct deltaNode
{
    uint32_t expectedEpoch;
    uint32_t delta;
    dqListNode_t node;
} deltaNode_t;

typedef struct deltaChain
{
    uint32_t currentEpoch;
    dq_t dq;
} deltaChain_t;

#define deltaNodeFromDqNode(dqNode) containerOf((dqNode), deltaNode_t, node)

inline
void deltaChainInit(deltaChain_t *chain)
{
    dqInit(&chain->dq);
}

inline
void deltaChainInsert(deltaChain_t *chain, deltaNode_t *node, uint32_t delta)
{
    dq_t *dq = NULL;
    dqListNode_t *cur = NULL;
    deltaNode_t *curNode = NULL;
    dqListNode_t *prev = NULL;
    
    dq = &chain->dq;
    cur = dq->front;
    
    node->expectedEpoch = chain->currentEpoch + delta;

    while (cur) {

        curNode = deltaNodeFromDqNode(cur);
        if (curNode->delta > delta) {
            curNode->delta -= delta;
            break;
        }

        delta -= curNode->delta;
        prev = cur;
        cur = cur->next;
    }
    
    node->delta = delta;
    dqInsertPrev(dq, prev, &node->node);
}

inline 
void deltaChainTick(deltaChain_t *chain)
{
    dq_t *dq = NULL;
    dqListNode_t *dqNode = NULL;
    deltaNode_t *node = NULL;

    dq = &chain->dq;
    dqNode = dq->front;

    chain->currentEpoch++;
    
    while (dqNode) {

        node = deltaNodeFromDqNode(dqNode);
        if (node->delta != 0) {
            node->delta--;
            return;
        }  
        
        dqNode = dqNode->next;
    }
}

inline 
deltaNode_t *deltaChainPopExpired(deltaChain_t *chain)
{
    dqListNode_t *dqNode = NULL;
    deltaNode_t *node = NULL;
    
    dqNode = chain->dq.front;
    if (!dqNode)
        return NULL;

    node = deltaNodeFromDqNode(dqNode);
    if (node->delta > 0)
        return NULL;

    dqPopFront(&chain->dq);
    return node;
}

inline
deltaNode_t *deltaChainPop(deltaChain_t *chain)
{
    dqListNode_t *dqNode = NULL;
    deltaNode_t *deltaNode =  NULL;
    dqListNode_t *nextdqNode = NULL;
    deltaNode_t *nextDeltaNode = NULL;

    dqNode = dqPopFront(&chain->dq);
    if (!dqNode)
        return NULL;

    deltaNode = deltaNodeFromDqNode(dqNode);

    nextdqNode = chain->dq.front;
    if (nextdqNode) {
        nextDeltaNode = deltaNodeFromDqNode(nextdqNode);
        nextDeltaNode->delta += deltaNode->delta;
    }

    return deltaNode;
}

inline 
bool deltaChainIsEmpty(deltaChain_t *chain)
{
    return dqIsEmpty(&chain->dq);
}

inline
deltaNode_t *deltaChainPeek(deltaChain_t *chain)
{
    dqListNode_t *dqNode = NULL;
  
    dqNode = chain->dq.front;

    return dqNode ? deltaNodeFromDqNode(dqNode) : NULL;
}

/* This is only safe when the expected epoch has not been passed */

inline 
uint32_t deltaChainTimeUntil(deltaChain_t *chain, deltaNode_t *node)
{
    return node->expectedEpoch - chain->currentEpoch;
}

inline
void deltaChainDequeue(deltaChain_t *chain, deltaNode_t *node)
{
    dqListNode_t *next = NULL;
    deltaNode_t *nextDeltaNode = NULL;

    next = node->node.next;
    if (next) {
        nextDeltaNode = deltaNodeFromDqNode(next);
        nextDeltaNode->delta += node->delta;
    }

    dqDequeue(&chain->dq, &node->node);
}

#endif