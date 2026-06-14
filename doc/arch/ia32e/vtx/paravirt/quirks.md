# Quirks

Documented list of quirks: <br>
- Emulator presents to the guest that the BIOS has locked the apic configuration to x2apic <br>
- Guest may still be able to execute enqcmd/s if supported by hardware but it should #GP rather than #UD <br>
- Vt-x will not be used on cores with tsx enabled (tsx will usually be left disabled by the BIOS or via a microcode update) <br>