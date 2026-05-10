# Deferrable Server (DS)

WorkhorseRT has support for deferrable server, where a task’s budget is replenished at fixed intervals defined by its period. When a task exhausts its budget, it is throttled until the next replenishment point, at which time its full budget becomes available again.

---

DS can be enabled through the config ```KSCHED_ALGORITHM_DS```.