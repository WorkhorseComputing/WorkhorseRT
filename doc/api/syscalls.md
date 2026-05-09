# Syscalls

All syscalls follow sysV ABI, and return a -errno on failure

---

```WORKHORSE_SYS_INVOCATION_CTRL 0```

param1 - ctrl <br>
param2 - val

Ctrls:

```WORKHORSE_INVOCATION_CTRL_DO_RETURN 0```

- Returns to the invoking domain and return address from the last entry in the calling tasks invocation stack.

```WORKHORSE_INVOCATION_CTRL_DO_IPC 1```

- Invokes the domain identified by the domain ID in val if authorized to do so. 
- It is up to the tasks to define their ABI for passing arguments between domains through invocations.
- It is the invoked domains responsibility to switch the stack of the invoking task.
- The calling task is trusting the invoked domain.

```WORKHORSE_INVOCATION_CTRL_GET_INVOCATIONS_AVAIL 2```

- Returns the number of free entries in the calling tasks invocation stack. 
- This should be used before carrying out operations which rely on catching exceptions, as in the case the invocation stack is exhausted and a task takes an exception, it transitions to failure state.

```WORKHORSE_INVOCATION_CTRL_GET_INVOKING_DOM_ID 3```

- Returns the domain ID of the domain the task was in when this domain was invoked, from the last entry in the calling tasks invocation stack.

```WORKHORSE_INVOCATION_CTRL_GET_INVOCATION_TYPE 4```

- Returns the invocation type from the last entry in the calling tasks invocation stack.

```WORKHORSE_INVOCATION_CTRL_GET_RETURN_ADDRESS 5```

- Returns the return address from the last entry in the calling tasks invocation stack if it was an exception.

```WORKHORSE_INVOCATION_CTRL_GET_VMEM_FAULT_ADDRESS 6```

- Returns the faulting address from the last entry in the calling tasks invocation stack if it was an exception.

```WORKHORSE_INVOCATION_CTRL_GET_ERROR_CODE 7```

- Returns the error code from the last entry in the calling tasks invocation stack if it was an exception.

```WORKHORSE_INVOCATION_CTRL_SET_RETURN_ADDRESS 8```

- Sets the return address of the last entry in the calling tasks invocation stack to val if it was an exception. 

---

```WORKHORSE_SYS_SCHED_CTRL 1```

param1 - ctrl

Ctrls:

```WORKHORSE_SCHED_CTRL_YIELD 0```

- Thread only
- Hands control to the scheduler 
- The calling task is not schedulable until the next tick.
- The calling task is still charged for the current tick.

```WORKHORSE_SCHED_CTRL_THROTTLE 1```

- Thread only
- Hands control to the scheduler and throttles the calling task.
- The calling task is not schedulable until the next tick.
- The calling task is still charged for the current tick.
- The calling task is immediately throttled on the next tick.
- Care must be taken, e.g., with DS, if the task is throttled on the next tick, its budget may be immediately replenished if its period expires on the same tick.

```WORKHORSE_SCHED_CTRL_LSR_DONE 2```

- LSR only
- Hands control to the scheduler and Transitions the calling LSR to a dormant state, where it is woken up on the next relevant hardware event

---

```WORKHORSE_SYS_GET_DOM_ID 2```

- Returns the calling tasks current domains ID.

---

```WORKHORSE_SYS_GET_TASK_ID 3```

- Returns the calling tasks task ID.

---

```WORKHORSE_SYS_GET_TASK_TYPE 4```

- Returns the calling tasks type.

Types:
- ```WORKHORSE_TASK_TYPE_THREAD 0```
- ```WORKHORSE_TASK_TYPE_LSR 1```
- ```WORKHORSE_TASK_TYPE_IDLE 2```

---

```WORKHORSE_SYS_GET_CPU_ID 5```

- Returns the current cpu ID.

---