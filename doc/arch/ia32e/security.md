# Security

## MMIO

The APIC MMIO region is reserved exclusively for the kernel. plugins must not provide access to this region to domains under any circumstances.

## Interrupts

On ia32e, any domain that communicates directly with hardware capable of generating interrupts becomes part of the Trusted Computing Base (TCB). Such domains must follow strict constraints on the interrupt vectors which are used to preserve temporal and spatial isolation.

The following vectors are reserved by the kernel and must never be targeted by device interrupts:
- ```IA32E_SPURIOUS_INT_VECTOR 255```
- ```IA32E_K_EVENT_VECTOR 254```
- ```IA32E_K_FAKE_ISR_VECTOR 253```

All other vectors may be used for device interrupts, however, not all vectors will support LSR monitoring.

## DMA

Although VT-d is currently not natively supported by WorkhorseRT, it can still be implemented and leveraged by a plugin to ensure devices aren't able to read/write to arbitrary memory.

A pointer to the RSDT is stored in ia32eGlobal, which can be retrieved by calling ```ia32eGlobal_t *ia32eGetGlobalPtr(void)```