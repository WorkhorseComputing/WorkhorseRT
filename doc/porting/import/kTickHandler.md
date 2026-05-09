# kTickHandler

```<import/kTickHandler.h>```

---

```c
void kTickHandler(void);
```

Routine which should be invoked in interrupt context, in response to a periodic tick event timer interrupt. This should be invoked only by the core identified by ```kCpuEventSenderFn_t kCpuEventSenderFn;```

---