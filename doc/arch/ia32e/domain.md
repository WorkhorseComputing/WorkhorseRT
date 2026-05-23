# Domain

ia32e domains are created with the following parameters:

- ```ia32ePml4_t *pml4BaseVirt;``` <br>
    Linear address of the domain’s PML4. pml4BaseVirt[511] is reserved for the kernel and must not be marked present. Plugins are responsible for populating the domain’s page tables and setting up its memory layout, including loading programs and configuring MMIO regions.

- ```uintptr_t pml4BasePhys;``` <br>
    Physical address of the domains pml4

- ```uint8_t iopb[8192];``` <br>
    IO port bitmap defining which IO ports can be accessed by tasks inside the domain