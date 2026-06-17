# kExceptionHandler

```<import/kExceptionHandler.h>```

---

```c
void kExceptionHandler(bool fail, kDomainInvocationType_t type, uintptr_t returnAddress, 
                       uintptr_t vmemFaultAddress, uintptr_t errorCode);
```

Routine which should be invoked in interrupt context in response to an exception.

fail = true will put the task into a failure state immediately.

---