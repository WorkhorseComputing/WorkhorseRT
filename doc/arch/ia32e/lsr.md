# LSR

On ia32e, interrupt vectors range from 0–255 and map to 16 priority levels (0–15), where 0 is the lowest priority and 15 the highest. Vectors at priority levels 0 and 15 cannot be monitored by LSRs, all other vectors may be monitored.

When an interrupt is delivered, the LSRs associated with that vector are woken, and the TPR is raised to that vector’s priority level so that only higher priority interrupts may preempt it. Once all LSRs for the current priority level have completed, the TPR is lowered to the next highest priority level still being serviced.