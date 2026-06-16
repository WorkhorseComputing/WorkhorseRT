# Msr

Workhorse allows guests to carry out operations that aren't natively handled by the emulator via synthetic msr's.

> **Note**:  Workhorse may not emulate certain msr's which are to be supported by the cpu model, guests should not blindly trust the cpu model in cpuid, and should only touch msr's who are associated with features explicitly enumerated via cpuid.

---

```IA32E_EMULATOR_SHUTDOWN   0x40000000```

- Write only.
- A write with a non zero value incurs a #GP(0).
- Broadcasts a shutdown to all vcpus in the guest, unlike ```WORKHORSE_SCHED_CTRL_FAILURE``` which only shuts down the calling vcpu.

---

```IA32E_EMULATOR_IPI   0x40000001```

- Write only.
- vector <= 15 & nmi = 0, invalid domain id, the domain not being a vm, unauthorised ipi or invalid dest incurs a #GP(0).
- Sends an ipi to the destinations x2apic, with the ability to send ipis across domains if explicitly authorised to via the senders invocation permission bitmap.
- [0:31] = domId
- [32:39] = dest
- [40:47] = vector
- [48] = nmi
- [49:63] = reserved

---