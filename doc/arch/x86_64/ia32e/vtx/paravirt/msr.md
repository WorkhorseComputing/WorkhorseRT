# Msr

Workhorse allows guests to carry out operations that aren't natively handled by the emulator via synthetic msr's.

> **Note**:  Workhorse may not emulate certain msr's which are to be supported by the cpu model, guests should not blindly trust the cpu model in cpuid, and should only touch msr's who are associated with features explicitly enumerated via cpuid.

---

```IA32E_EMULATOR_SHUTDOWN   0x40000000```

- Write only.
- A write with a non zero value incurs a #GP(0).
- Broadcasts a shutdown to all vcpus in the guest, unlike ```WORKHORSE_SCHED_CTRL_FAILURE``` which only shuts down the calling vcpu.

---