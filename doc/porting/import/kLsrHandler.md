# kLsrHandler

```<import/kLsrHandler.h>```

---

```c
typedef enum kLsrHandlerOp
{
    K_LSR_OP_INVALID =          0,
    K_LSR_OP_PUSH =             1,
    K_LSR_OP_PUSH_CURRENT =     2,
    K_LSR_OP_RESCHEDULE =       3
} kLsrHandlerOp_t;

void kLsrHandler(kLsrHandlerOp_t op, kSchedLsr_t *lsr);
```

Routine which should be invoked in response to an LSR being woken up.

```K_LSR_OP_PUSH```

- Pushes an LSR to the LSR stack

```K_LSR_OP_PUSH_CURRENT```

- Pushes the current LSR to the LSR stack, if the running task is not an LSR, it will be marked as pending to run

 ```K_LSR_OP_RESCHEDULE```

- Must be invoked if ```K_LSR_OP_PUSH_CURRENT``` was invoked!!
- If an LSR is currently running, it will continue running
- If a task which is not of type LSR is running, and an LSR is waiting in the LSR stack, the LSR will preempt the task
- If no task is running, and an LSR is waiting, the LSR will start running
- A non LSR task preempted by an LSR will still be charged for the current tick

---