# Cyclic

WorkhorseRT provides support for cyclic scheduling, where tasks are assigned to frames in a fixed, repeating schedule. The scheduler executes each frame sequentially, advancing to the next frame assigned to a ready task. A task may appear in multiple frames, allowing it to run at multiple points within the major cycle.

When a frame becomes active, the scheduler selects the task assigned to that frame, runs it for its timeslice, and then advances to the next frame once the timeslice expires. This policy can be used to implement cyclic executives.

---

Cyclic scheduling can be enabled via the config ```KSCHED_POLICY_CYCLIC```, the number of available frames can be configured via ```KSCHED_POLICY_CYCLIC_NUM_FRAMES```.