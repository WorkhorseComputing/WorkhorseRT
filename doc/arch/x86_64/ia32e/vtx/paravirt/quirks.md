# Quirks

Documented list of quirks: <br>
- Emulator presents to the guest that the BIOS has locked the apic configuration to x2apic <br>
- Guest may still be able to execute enqcmd/s if supported by hardware but it should #GP rather than #UD <br>
- Silent fails on invalid writes to IA32E_BIOS_UPDT_TRIG instead of #GP(0) <br>
- Vt-x will not be used on cores with tsx enabled (tsx will usually be left disabled by the BIOS or via a microcode update) <br>
- Msrs which are to be supported by specific cpu models may not be emulated, guests should not blindly trust the cpu version but only features explicitly enumerated via cpuid <br>