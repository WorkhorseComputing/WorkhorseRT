# Round Robin (RR)

WorkhorseRT provides support for round robin scheduling. 

The RR scheduler selects the highest priority ready task, resets its timeslice, and executes it until the timeslice expires. When the timeslice is expired, the scheduler switches to the next ready task at the same priority. the RR scheduler implements a fixed priority preemptive scheduling policy (FPPS) to ensuring higher priority tasks always preempt lower priority tasks.

---

RR can be enabled via the config ```KSCHED_POLICY_RR```, the number of available priority levels can be configured via ```KSCHED_POLICY_RR_NUM_PRIO```.