# kPlugin

```<plugin/kPlugin.h>```

Plugins may use all APIs specified in kPlugin.h, aswell as architecture specific APIs if stated safe to do so. 

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