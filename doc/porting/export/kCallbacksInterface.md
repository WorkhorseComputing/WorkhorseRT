# kCallbackInterface

```<export/kCallbackInterface.h>```

Registering this interface is optional.

---

```c
typedef void (*kCallbackActivationFn_t)(kSchedTask_t *task);
typedef void (*kCallbackResponseFn_t)(kSchedTask_t *task);
typedef void (*kCallbackCpuHandoffFn_t)(void);

typedef struct kCallbackOps
{
    kCallbackActivationFn_t kCallbackActivationFn;
    kCallbackResponseFn_t kCallbackResponseFn;
    kCallbackCpuHandoffFn_t kCallbackCpuHandoffFn;
} kCallbackOps_t;
```

--- 

```c
int kCallbackOpsInit(kCallbackOps_t *ops);
```

Registers the kCallbackOps interface.

---

```c
void kCallbackActivation(kSchedTask_t *task);
```

Called everytime a task is transitioned from a non schedulable state to a schedulable state.

(Prior to gPluginsDone being set, this can be called on the bsp, otherwise the core in which
the scheduler corresponds to)

---

```c
void kCallbackResponse(kSchedTask_t *task);
```

Called everytime a task is transitioned from a schedulable state to a non schedulable state.

This will be called on the core in which the scheduler corresponds to

---

```c
void kCallbackCpuHandoff(void);
```

Called on each core right before cpu handoff is committed.

---