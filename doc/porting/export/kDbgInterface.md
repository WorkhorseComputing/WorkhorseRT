# kDbgInterface

```<export/kDbgInterface.h>```

Registering this interface is optional.

---

```c
typedef void (*kDbgStrFn_t)(const char *str);

typedef struct kDbgOps
{
    kDbgStrFn_t kDbgStrFn;
} kDbgOps_t;
```

---

```c
int kDbgOpsInit(kDbgOps_t *ops);
```

Registers the kDbgOps interface.

---

```kDbgStrFn_t kDbgStrFn;```

Utility function for logging a given string.

---