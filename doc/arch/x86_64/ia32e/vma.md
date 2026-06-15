# VMA

```<ia32eVma.h>```

Plugins may use the APIs provided by ia32eVma.h to map regions of memory into the kernels address space.

--- 

```c
IA32E_DEFINE_VMA_PARTITION_TABLE(name, count)
```

Defines a vma partition table.

---

```c
IA32E_REGISTER_VMA_PARTITION_TABLE(_name, _tablePhys, _count, descriptorPtr)
```

Registers a vma partition table to be initialized during boot, and populates a descriptor to detail initialization information about the partition table.

---

```c
ia32eVirtToPhysStatic(addr)
```

Converts a virtual address of some statically defined data to a physical address.

---

```c
IA32E_VMA_PARTITION_PENTRY_GET(partition, pentry)
```

Returns a pointer to the corresponding page table entry for a vma partition.

---

```c
IA32E_VMA_PARTITION_TABLE_IDX_GET(table, idx)
```

Returns a pointer to the corresponding vma partition at the passed index of the vma partition table.

---

```c
IA32E_VMA_PARTITION_TABLE_ENTRY_TO_IDX(entry)
```

Returns an index to the corresponding vma partition for an index to a page table entry in the vma partition table.

---

```c
IA32E_VMA_PARTITION_TABLE_ENTRY_TO_PENTRY(entry)
```

Returns an index to the page table entry of the corresponding vma partition from an index to a page table entry in the vma partition table.

---

```c
void *ia32eVmaEarlyMapRange(uintptr_t base, size_t size, bool io);
```

Maps a region of memory into the kernels virtual address space via the vma partition table reserved by the kernel.

---

```c
void ia32eVmaEarlyRemapPg(uintptr_t pgBase, uintptr_t base, bool io);
```

Remaps a page already mapped into the kernels virtual address space via the vma partition table reserved by the kernel.

This must not be called on any pages which are not mapped via ```ia32eVmaEarlyMapRange```.

---