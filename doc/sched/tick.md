# Tick

WorkhorseRT uses a tick‑driven scheduler, where on every tick, forward progress is made once all cores have received and processed the tick event. 

This improves determinism, as tick‑based schedulers provide a constant, predictable heartbeat, whereas tickless schedulers introduce variable latency. Tickless approaches can offer better power efficiency, but tick‑based scheduling provides stronger timing guarantees than tickless approaches and greater flexibility compared to cycle‑driven approaches.

---

Plugins can register callbacks to be notified when tasks are scheduled in and scheduled out via the following structure, passed in ```kPluginTaskThreadParam_t``` during thread creation and ``` kPluginTaskLsrParam_t``` during LSR creation:

```c
typedef void (*kSchedTaskCallbackFn_t)(kSchedTask_t *task);

typedef struct kSchedTaskCallbacks
{
    kSchedTaskCallbackFn_t tickCallbackFn;
    kSchedTaskCallbackFn_t inCallbackFn;
    kSchedTaskCallbackFn_t outCallbackFn;
    kSchedTaskCallbackFn_t activationCallbackFn;
    kSchedTaskCallbackFn_t responseCallbackFn;
} kSchedTaskCallbacks_t;
```

This is essential for implementing execution models focused on improving determinism and power savings such as PREM and shared slack reclamation, which often rely on knowing which task is currently running.

---

- [Throttling algorithms](algorithm)
- [Policies](policy)