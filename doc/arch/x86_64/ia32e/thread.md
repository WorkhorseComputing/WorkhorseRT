# Thread

On ia32e, threads are initialized with the parameter - ```uint8_t tpr```.

The tpr must be a value between 0 and 14, if it is 0, the kernel will initialize the thread with a tpr of 1. 

The kernel will ensure that on each core, the hardware tpr is set to the value of the highest tpr associated with a
task associated with that core that is in a schedulable state. E.g., if a task is throttled or dormant, it is not in 
a schedulable state, if a task is ready, running, pending or has a deferred tick it is in a schedulable state. 

Interrupts will only arrive if their priority is greater than the hardware tpr.

This mechanism is implemented to allow for plugins to prioritise specific tasks over certain interrupts. Without this,
an interrupt storm which constantly causes LSRs to be woken up can cause the current thread to take longer to finish
its timeslice due to it not being charged for ticks that it never ran in, which is something to consider with LSRs
that can run for longer than a tick at a time. The alternative is to implement mitigations for this problem at the 
application level, but this can be inconvenient. 

Refer to [Security](security.md) for advice on mitigating other DOS attacks that can be carried out by compromised
devices.