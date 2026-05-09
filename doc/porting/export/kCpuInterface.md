# kCpuInterface

```<export/kCpuInterface.h>```

---

```c
typedef void (*kCpuInvokeRoutineFn_t)(void);

typedef uint32_t (*kThisCpuIdFn_t)(void);
typedef void (*kCpuInvokeAllRendezvousFn_t)(kCpuInvokeRoutineFn_t fn);
typedef void (*kCpuSelfIpiFn_t)(kCpuInvokeRoutineFn_t fn);
typedef void (*kCpuTaskIdleCtxInitFn_t)(kSchedTask_t *task);
typedef void (*kCpuTaskCtxInitFn_t)(kSchedTask_t *task, uintptr_t pc);
typedef void (*kCpuTaskSaveCtxFn_t)(kSchedTask_t *task);
typedef void (*kCpuTaskRestoreCtxFn_t)(kSchedTask_t *task);
typedef uintptr_t (*kCpuSyscallGetReturnAddressFn_t)(void);
typedef void (*kCpuSyscallSetReturnAddressFn_t)(uintptr_t returnAddress);
typedef void (*kCpuExceptionSetReturnAddressFn_t)(uintptr_t returnAddress);
typedef void (*kCpuEnterDomainFn_t)(kDomain_t *domain);
typedef void (*kCpuTaskLsrPushFn_t)(kSchedTask_t *task);
typedef uint32_t (*kCpuEventSenderFn_t)(void);
typedef bool (*kCpuIdValidateFn_t)(uint32_t cpuId);
typedef int (*kCpuLsrInfoInitFn_t)(archSchedLsrInfo_t *info, archSchedLsrParam_t *param);
typedef int (*kCpuDomainInfoInitFn_t)(archDomainInfo_t *info, archDomainParam_t *param);

typedef struct kCpuOps
{
    kThisCpuIdFn_t kThisCpuIdFn;
    kCpuInvokeAllRendezvousFn_t kCpuInvokeAllRendezvousFn;
    kCpuSelfIpiFn_t kCpuSelfIpiFn;
    kCpuTaskIdleCtxInitFn_t kCpuTaskIdleCtxInitFn;
    kCpuTaskCtxInitFn_t kCpuTaskCtxInitFn;
    kCpuTaskSaveCtxFn_t kCpuTaskSaveCtxFn;
    kCpuTaskRestoreCtxFn_t kCpuTaskRestoreCtxFn;
    kCpuSyscallGetReturnAddressFn_t kCpuSyscallGetReturnAddressFn;
    kCpuSyscallSetReturnAddressFn_t kCpuSyscallSetReturnAddressFn;
    kCpuExceptionSetReturnAddressFn_t kCpuExceptionSetReturnAddressFn;
    kCpuEnterDomainFn_t kCpuEnterDomainFn;
    kCpuTaskLsrPushFn_t kCpuTaskLsrPushFn;
    kCpuEventSenderFn_t kCpuEventSenderFn;
    kCpuIdValidateFn_t kCpuIdValidateFn;
    kCpuLsrInfoInitFn_t kCpuLsrInfoInitFn;
    kCpuDomainInfoInitFn_t kCpuDomainInfoInitFn;
    
} kCpuOps_t;
```

---

```c
int kCpuOpsInit(kCpuOps_t *ops);
```

Registers the kCpuOps interface.

---

```kThisCpuIdFn_t kThisCpuIdFn;```

Returns the calling cpus cpu Id.

cpu Id's must be numbered from 0 - (CONFIG_KMAX_CPUS - 1).

---

```kCpuInvokeAllRendezvousFn_t kCpuInvokeAllRendezvousFn;```

Executes a routine on the current core, puts every other core into interrupt context and executes the routine on those (e.g., using an interprocessor interrupt). Returns once every core has finished executing the routine.

---

```kCpuSelfIpiFn_t kCpuSelfIpiFn;```

Puts the current core into interrupt context and executes a routine.

---

```kCpuTaskIdleCtxInitFn_t kCpuTaskIdleCtxInitFn;```

Initializes the context of a task of idle type.

---

```kCpuTaskCtxInitFn_t kCpuTaskCtxInitFn;```

Initializes the context of a task of thread or LSR type.

---

```kCpuTaskSaveCtxFn_t kCpuTaskSaveCtxFn;```

Saves the context of the currently running task. This is called in interrupt context.

---

```kCpuTaskRestoreCtxFn_t kCpuTaskRestoreCtxFn;```

Restores the context of a given task. This is called in interrupt context.

---

```kCpuSyscallGetReturnAddressFn_t kCpuSyscallGetReturnAddressFn;```

Gets the return address of a syscall.

---

```kCpuSyscallSetReturnAddressFn_t kCpuSyscallSetReturnAddressFn;```

Sets the return address of a syscall.

---

```kCpuExceptionSetReturnAddressFn_t kCpuExceptionSetReturnAddressFn;```

Sets the return address of an exception. This is called in interrupt context.

---

```kCpuEnterDomainFn_t kCpuEnterDomainFn;```

Enters a given domain.

---

```kCpuTaskLsrPushFn_t kCpuTaskLsrPushFn;```

Pushes an LSR to start monitoring for a corresponding event.

---

```kCpuEventSenderFn_t kCpuEventSenderFn;```

Returns the cpu Id of the core who sends tick events.

---

```kCpuIdValidateFn_t kCpuIdValidateFn;```

Checks a given cpu Id is valid and online.

---

```kCpuLsrInfoInitFn_t kCpuLsrInfoInitFn;```

Initializes the archSchedLsrInfo field of an LSR.

returns -errno on failure.

---

```kCpuDomainInfoInitFn_t kCpuDomainInfoInitFn;```

Initializes the archDomainInfo field of a Domain.

returns -errno on failure.

---