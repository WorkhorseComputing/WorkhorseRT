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

#ifndef _IA32E_VMA_H_
#define _IA32E_VMA_H_

#include <ia32e.h>
#include <ia32eAsm.h>
#include <generated/autoconf.h>

#define IA32E_VMA_PARTITION_REGISTRY ".ia32eVma"
#define __ia32eVmaPartitionRegistry __attribute__ ((section(IA32E_VMA_PARTITION_REGISTRY), used, aligned(1)))

#define IA32E_VMA_PARTITION_ENTRY_COUNT IA32E_PT_ENTRY_COUNT
#define IA32E_VMA_PARTITION_SIZE (IA32E_VMA_PARTITION_ENTRY_COUNT * IA32E_PAGE_SIZE_2MB)

STATIC_ASSERT(IA32E_FREE_PDPTE_START + CONFIG_X86_64_IA32E_MAX_VMA_PARTITIONS <= IA32E_PT_ENTRY_COUNT);
STATIC_ASSERT(CONFIG_X86_64_IA32E_VMA_EARLY_PARTITION_TABLE_COUNT <= CONFIG_X86_64_IA32E_MAX_VMA_PARTITIONS);

typedef struct ia32eVmaDescriptor
{
    uintptr_t addr;
    size_t size;
    unsigned long numValid;
} ia32eVmaDescriptor_t;

typedef struct ia32eVmaPartition
{ 
    ia32ePde_t pde[IA32E_VMA_PARTITION_ENTRY_COUNT];
} ia32eVmaPartition_t;
SIZE_ASSERT(ia32eVmaPartition_t, 4096);

typedef struct ia32eVmaPartitionTicket
{
    const char *name;
    uintptr_t tablePhys;
    unsigned long count;
    ia32eVmaDescriptor_t *descriptor;
} ia32eVmaPartitionTicket_t;

extern ia32eVmaPartitionTicket_t __ia32eVmaStart[];
extern ia32eVmaPartitionTicket_t __ia32eVmaEnd[];

#define ia32eVmaCount() (((uintptr_t)__ia32eVmaEnd - (uintptr_t)__ia32eVmaStart) / sizeof(ia32eVmaPartitionTicket_t))

#define IA32E_DEFINE_VMA_PARTITION_TABLE(name, count) ia32eVmaPartition_t ATTR_ALIGNED(4096) (name)[(count)]
#define IA32E_VMA_PARTITION_TABLE_COUNT(x) ARRAY_LEN((x))

#define IA32E_VMA_PARTITION_PENTRY_GET(partition, pentry) (&(partition)->pde[(pentry)])

#define IA32E_VMA_PARTITION_TABLE_IDX_GET(table, idx) (&(table)[(idx)])
#define IA32E_VMA_PARTITION_TABLE_ENTRY_TO_IDX(entry) ((entry) / IA32E_VMA_PARTITION_ENTRY_COUNT)
#define IA32E_VMA_PARTITION_TABLE_ENTRY_TO_PENTRY(entry) ((entry) % IA32E_VMA_PARTITION_ENTRY_COUNT)

#define IA32E_REGISTER_VMA_PARTITION_TABLE(_name, _tablePhys, _count,       \
                                           descriptorPtr)                   \
    __ia32eVmaPartitionRegistry                                             \
    static                                                                  \
    const ia32eVmaPartitionTicket_t vmaPartitionTicket_##_name = {          \
        .name = (#_name),                                                   \
        .tablePhys = (_tablePhys),                                          \
        .count = (_count),                                                  \
        .descriptor = (descriptorPtr),                                      \
    }

#define ia32eVirtToPhysStatic(addr) ((uintptr_t)(addr) - IA32E_KERNEL_OFFSET)
#define ia32ePhysToVirtStatic(addr) ((uintptr_t)(addr) + IA32E_KERNEL_OFFSET)

bool ia32eVmaTicketService(ia32eVmaPartitionTicket_t *ticket);
void ia32eVmaInit(void);

int32_t ia32eVmaEarlyFindFreeRange(uint32_t numPages);
void ia32eVmaEarlyRemap(uint32_t entry, uintptr_t physBase, bool io, uint32_t numPages);
void *ia32eVmaEarlyMapRange(uintptr_t base, size_t size, bool io);
void ia32eVmaEarlyRemapPg(uintptr_t pgBase, uintptr_t base, bool io);

#endif