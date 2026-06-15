# Hypervisor

On ia32e, hardware assisted virtualization can be used if the ```IA32E_VTX``` configuration is enabled. Any guest OS' do not need to be paravirtualized so long as the plugins which initialize their domains define emulation callbacks to handle emulating peripherals which the guest OS requires to run unmodified.

This feature is supported on platforms with support for vt-x, EPTs, and unrestricted guest.

Virtual machines making use of hardware assisted virtualisation are restricted in their IPC capabilities, they cannot be invoked, nor can their vcpus carry out invocations.

- [Vtx documentation](vtx)