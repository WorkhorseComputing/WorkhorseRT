# Task

Both threads and LSRs can be apart of virtual machines.

For tasks that are initialized in domains that are virtual machines, the following parameter is required:

```c
typedef struct ia32eVtxParam
{
    ia32eVtxVmcsRegion_t *vmcsVirt;
    uintptr_t vmcsPhys;
    ia32eEmulatorCallbacks_t callbacks;
} ia32eVtxParam_t;
```

- vmcsVirt and vmcsPhys must be aligned to 4kb and must be passed as parameters. 
- callbacks are optional.

---

Plugins can register callbacks for certain events in the guest, this allows them to emulate peripherals which the guest OS expects to be available, allowing for them to run without modification. Callbacks can should use helpers available in ```<ia32eEmulator.h>```.

```c
typedef struct ia32eEmulatorCallbacks
{
    ia32eEmulatorInOutCallbackFn_t ia32eEmulatorInOutCallbackFn;
    ia32eEmulatorEptFaultCallbackFn_t ia32eEmulatorEptFaultCallbackFn;
    ia32eEmulatorEptMisconfigCallbackFn_t ia32eEmulatorEptMisconfigCallbackFn;
} ia32eEmulatorCallbacks_t;
```

Callbacks: <br>
```ia32eEmulatorInOutCallbackFn``` - Called during a trapped PMIO access. <br>
```ia32eEmulatorEptFaultCallbackFn``` - Called when a vcpu accesses memory such that is not permissible in its EPTs. <br>
```ia32eEmulatorEptMisconfig``` - Called when a vcpus EPTs is configured incorrectly. <br>

Return values: <br>
```IA32E_EMULATOR_CALLBACK_FAILURE``` - The event was unhandled <br> 
```IA32E_EMULATOR_CALLBACK_SUCCESS``` - The event was handled successfully <br>

---