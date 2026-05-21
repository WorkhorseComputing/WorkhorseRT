# kPlugin

```<plugin/kPlugin.h>```

Plugins may use all APIs specified in kPlugin.h, aswell as architecture specific APIs if stated safe to do so. 

--- 

```c
K_REGISTER_PLUGIN(_name, _fn, order)
```

Registers a plugin. 

Plugins run sequentially in the order derived by the order parameter.
Plugins which have the same order value are ordered arbitrarily, but will still run before plugins of a higher order.

---

```c
kPluginPrintf(fmt, ...)
```

Formatted printing utility.

---

```c
int kPluginInitTaskThread(kSchedTask_t *task, kPluginTaskThreadParam_t *param);
```

Initializes a task of type thread using the parameters supplied by the plugin.

---

```c 
int kPluginInitTaskLsr(kSchedTask_t *task, kPluginTaskLsrParam_t *param);
```

Initializes a task of type LSR using the parameters supplied by the plugin.

---

```c
int kPluginInitDomain(kDomain_t *domain, kPluginDomainParam_t *param);
```

Initializes a domain, including its configuration defined by the plugin.

---

```c
int kPluginInitIdleCallbacks(uint32_t cpuId, kSchedTaskCallbacks_t *callbacks);
```

Attaches event callbacks to the idle task of a given cpu.

---