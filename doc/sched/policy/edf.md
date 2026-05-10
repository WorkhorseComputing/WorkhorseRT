# Earliest Deadline First (EDF)

WorkhorseRT’s EDF scheduler uses relative deadlines. Each task is assigned a relative deadline, and the scheduler always selects the task whose deadline will expire the soonest. When a task’s relative deadline expires, it is reset the next time the task becomes ready. 

Care must be taken to ensure that a task’s deadline is properly reset upon each activation. This can be achieved by assigning the task a suitable budget and period such that it can meet its deadline, while also ensuring the period is long enough that the task does not reuse a stale deadline from before it was last throttled.

---

EDF can be enabled via the config ```KSCHED_POLICY_EDF```.