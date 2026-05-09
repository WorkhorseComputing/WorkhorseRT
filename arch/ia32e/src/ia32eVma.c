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

#include <ia32eVma.h>
#include <ia32eCpu.h>
#include <lib/dsa/bitmap.h>
#include <errno.h>
#include <stdWorkhorse.h>

static 
ia32ePdpte_t *ia32ePdptReloc = NULL;

static 
unsigned long vmaPartitionCount = 0;

static 
ia32eVmaDescriptor_t ia32eVmaEarlyDesc = {0};

static 
IA32E_DEFINE_VMA_PARTITION_TABLE(ia32eVmaEarlyPartitionTable, 
                                CONFIG_IA32E_VMA_EARLY_PARTITION_TABLE_COUNT);

IA32E_REGISTER_VMA_PARTITION_TABLE(ia32eVmaEarlyPartitionTable, 
                                   ia32eVirtToPhysStatic(ia32eVmaEarlyPartitionTable),
                                   IA32E_VMA_PARTITION_TABLE_COUNT(ia32eVmaEarlyPartitionTable),
                                   &ia32eVmaEarlyDesc);

static 
uint64_t ia32eVmaEarlyClock = 0;

static 
uint64_t ia32eVmaEarlyEntries = 0;

bool ia32eVmaTicketService(ia32eVmaPartitionTicket_t *ticket)
{
    unsigned long oldCount = 0;
    unsigned long count = 0;
    unsigned long numValid = 0;
    unsigned long i = 0;
    size_t offset = 0;

    oldCount = vmaPartitionCount;
    count = ticket->count;

    for (i = 0; i < count; i++) {

        ia32ePdptReloc[IA32E_FREE_PDPTE_START + vmaPartitionCount] = ticket->tablePhys | 0x3;
        vmaPartitionCount++;

        if (vmaPartitionCount == CONFIG_IA32E_MAX_VMA_PARTITIONS)
            break;
    }

    numValid = vmaPartitionCount - oldCount;
    offset = (IA32E_FREE_PDPTE_START + oldCount) *  IA32E_VMA_PARTITION_SIZE;

    ticket->descriptor->addr = numValid != 0 ? IA32E_KERNEL_OFFSET + offset : 0;
    ticket->descriptor->numValid = numValid;
    ticket->descriptor->size = numValid * IA32E_VMA_PARTITION_SIZE;

    return vmaPartitionCount == CONFIG_IA32E_MAX_VMA_PARTITIONS;
}

void ia32eVmaInit(void)
{
    uint32_t count = 0;
    uint32_t i = 0;
    ia32eVmaPartitionTicket_t *ticket = NULL;

    ia32ePdptReloc = (ia32ePdpte_t *)((uintptr_t)ia32ePdpt + IA32E_KERNEL_OFFSET);

    count = ia32eVmaCount();  
    for (i = 0; i < count; i++) {

        ticket = &__ia32eVmaStart[i];
        if (ia32eVmaTicketService(ticket))
            break;
    } 

    ia32eVmaEarlyEntries = ia32eVmaEarlyDesc.numValid * IA32E_VMA_PARTITION_ENTRY_COUNT;
}

int32_t ia32eVmaEarlyFindFreeRange(uint32_t numPages)
{
    int32_t ret = 0;

    if (numPages == 0)
        return -EINVAL;

    if ((ia32eVmaEarlyClock + numPages) > ia32eVmaEarlyEntries)
        return -ENOMEM;

    ret = ia32eVmaEarlyClock;
    ia32eVmaEarlyClock += numPages;

    return ret;
}

void ia32eVmaEarlyRemap(uint32_t entry, uintptr_t physBase, bool io, uint32_t numPages)
{
    uint32_t i = 0;
    uint32_t idx = 0;
    uint32_t pentry = 0;
    uintptr_t phys = 0;
    ia32eVmaPartition_t *partition = NULL;
    ia32ePde_t *pde = NULL;
    ia32ePde_t newPde = 0;

    physBase &= ~(IA32E_PAGE_SIZE_2MB - 1);

    for (i = 0; i < numPages; i++) {
        idx = IA32E_VMA_PARTITION_TABLE_ENTRY_TO_IDX(entry + i);
        pentry = IA32E_VMA_PARTITION_TABLE_ENTRY_TO_PENTRY(entry + i);
        phys = physBase + (i * IA32E_PAGE_SIZE_2MB);

        partition = IA32E_VMA_PARTITION_TABLE_IDX_GET(ia32eVmaEarlyPartitionTable, idx);
        pde = IA32E_VMA_PARTITION_PENTRY_GET(partition, pentry);

        newPde |= IA32E_PG_ENTRY_PRESENT_MASK;
        newPde |= IA32E_PG_ENTRY_RW_MASK;
        newPde |= IA32E_PG_ENTRY_PS_MASK;
        newPde |= IA32E_PG_ENTRY_GLOBAL_MASK;
        newPde |= phys;

        if (io) {
            newPde |= IA32E_PG_ENTRY_PWT_MASK;
            newPde |= IA32E_PG_ENTRY_PCD_MASK;
        }

        WRITE_ONCE(*pde, newPde);
    }

    barrier();
    __ia32eCr4ReenablePge();
}

void *ia32eVmaEarlyMapRange(uintptr_t base, size_t size, bool io)
{
    uint32_t numPages = 0;
    uintptr_t oldBase = 0;
    uint32_t pgOffset = 0;
    int32_t entry = 0;
    uint64_t addrOffset = 0;
    uintptr_t addr = 0;

    oldBase = base;

    base &= ~(IA32E_PAGE_SIZE_2MB - 1);
    pgOffset = oldBase - base;

    size = ((size + pgOffset) + (IA32E_PAGE_SIZE_2MB - 1)) & ~(IA32E_PAGE_SIZE_2MB - 1);
    numPages = size / IA32E_PAGE_SIZE_2MB;

    entry = ia32eVmaEarlyFindFreeRange(numPages); 
    if (entry < 0)
        return NULL;

    addrOffset = ((uint64_t)entry * IA32E_PAGE_SIZE_2MB);
    addr = (ia32eVmaEarlyDesc.addr + addrOffset) + pgOffset;

    ia32eVmaEarlyRemap(entry, base, io, numPages);

    return (void *)addr;
}

void ia32eVmaEarlyRemapPg(uintptr_t pgBase, uintptr_t base, bool io)
{
    uint32_t pdIdx = 0;
    uint32_t pdptIdx = 0;
    ia32ePdpte_t pdpte = 0;
    ia32ePde_t *pd = NULL;
    ia32ePde_t newPde = 0;

    pgBase &= ~(IA32E_PAGE_SIZE_2MB - 1);
    base &= ~(IA32E_PAGE_SIZE_2MB - 1);

    pdIdx = (pgBase >> IA32E_PAGE_SHIFT_2MB) & 0x1ff;
    pdptIdx = (pgBase >> IA32E_PAGE_SHIFT_1GB) & 0x1ff;

    pdpte = ia32ePdptReloc[pdptIdx] &~ (IA32E_PAGE_SIZE_4KB - 1);
    pd = (void *)(ia32ePhysToVirtStatic((uintptr_t)(pdpte)));

    newPde |= IA32E_PG_ENTRY_PRESENT_MASK;
    newPde |= IA32E_PG_ENTRY_RW_MASK;
    newPde |= IA32E_PG_ENTRY_PS_MASK;
    newPde |= IA32E_PG_ENTRY_GLOBAL_MASK;
    newPde |= base;

    if (io) {
        newPde |= IA32E_PG_ENTRY_PWT_MASK;
        newPde |= IA32E_PG_ENTRY_PCD_MASK;
    }

    WRITE_ONCE(pd[pdIdx], newPde);

    barrier();
    __ia32eCr4ReenablePge();
}