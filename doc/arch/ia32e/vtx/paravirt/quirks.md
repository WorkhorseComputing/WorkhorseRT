# Quirks

Documented list of quirks:
- Guest may still be able to execute enqcmd/s if supported by hardware but it should #GP rather than #UD
- Vt-x will not be used on cores with tsx enabled (tsx will usually be left disabled by the BIOS or via a microcode update)