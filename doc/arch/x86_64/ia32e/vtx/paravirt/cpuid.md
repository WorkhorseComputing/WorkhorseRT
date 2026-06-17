# Cpuid

Workhorse presents information to guests via cpuid leaves

---

```IA32E_EMULATOR_CPUIDV_START 0x40000000```

- Returns hypervisor identification info

eax - maximum supported cpuid leaf
ebx - 0x6b726f77
ecx - 0x73726f68
edx - 0x78747665

--- 

```IA32E_EMULATOR_CPUIDV_EMULATION 0x40000002```

- Returns hypervisor emulation info

edx: <br>
0 - gp0 on denied access to pmio or physical memory <br>
1 - task gate emulation supported <br>
2 - cr0.ne is not clamped to 1 <br>
3 - cr0.cd is not clamped to 0 <br>
4 - cr0.nw is not clamped to 0 <br>
5 - guest can execute hypercalls otherwise #GP <br>

--- 