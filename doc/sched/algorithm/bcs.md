# Burst-Cooldown Server (BCS)

Burst-cooldown server is a throttling algorithm implemented in workhorseRT designed for bursty workloads that occasionally need short spikes of CPU time whilst respecting long term limits.

It works by allowing a task to freely consume its budget, once its budget expires, it will be throttled and its period is restarted. When its period expires, its budget its be replenished.

BCS is more conservative than algorithms such as Sporadic Server or Deferrable Server, and developers should be aware that this can lead to under‑utilization in certain scenarios.

---

BCS is designed to satisfy the following invariant:

```
T = task
Q = budget
P = period

If Q <= P:
    ticks consumed by T in any window of length P <= Q
Else:
    ticks consumed by T in any window of length (Q + P) <= Q

```

---

BCS can be enabled through the config ```KSCHED_ALGORITHM_BCS```.