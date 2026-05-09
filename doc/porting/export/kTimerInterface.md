# kTimerInterface

```<export/kTimerInterface.h>```

---

```c
typedef uint32_t (*kTimerFrequencyHzFn_t)(void);
typedef void (*kTimerArmPeriodicFn_t)(uint32_t ticks);

typedef struct kTimerOps
{
    kTimerFrequencyHzFn_t kTimerFrequencyHzFn;
    kTimerArmPeriodicFn_t kTimerArmPeriodicFn;
} kTimerOps_t;
```

---

```c
int kTimerOpsInit(kTimerOps_t *ops);
```

Registers the kTimerOps interface.

---

```kTimerFrequencyHzFn_t kTimerFrequencyHzFn;```

Returns the frequency (Hz) of the tick event timer.

---

```kTimerArmPeriodicFn_t kTimerArmPeriodicFn;```

Arms the tick event timer to interrupt periodically at a given rate specified by the number of timer ticks.

The tick event timer must go to one specific core, which is identified through ```kCpuEventSenderFn_t kCpuEventSenderFn;```

---