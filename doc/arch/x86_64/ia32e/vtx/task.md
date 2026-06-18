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

Plugins can register callbacks for certain events in the guest, this allows them to easily extend the hypervisors functionality aswell as emulate peripherals which the guest OS expects to be available, allowing for them to run without modification. Callbacks can should use helpers available in ```<ia32eEmulator.h>```.

```c
typedef bool (*ia32eEmulatorCallbackFn_t)(ia32eVmexitRegs_t *regs);

typedef void (*ia32eEmulatorX2apicIrrCallbackFn_t)(uint8_t vector);
typedef void (*ia32eEmulatorX2apicIsrCallbackFn_t)(uint8_t vector);
typedef void (*ia32eEmulatorX2apicEoiCallbackFn_t)(uint8_t vector);
typedef bool (*ia32eEmulatorX2apicReadTmrCallbackFn_t)(uint8_t idx);

typedef struct ia32eEmulatorCallbacks
{
    ia32eEmulatorCallbackFn_t ia32eEmulatorInOutCallbackFn;
    ia32eEmulatorCallbackFn_t ia32eEmulatorRdmsrCallbackFn;
    ia32eEmulatorCallbackFn_t ia32eEmulatorWrmsrCallbackFn;
    ia32eEmulatorCallbackFn_t ia32eEmulatorEptFaultCallbackFn;
    ia32eEmulatorCallbackFn_t ia32eEmulatorEptMisconfigCallbackFn;
    ia32eEmulatorCallbackFn_t ia32eEmulatorRegsResetCallbackFn;

    struct
    {
        ia32eEmulatorX2apicIrrCallbackFn_t ia32eEmulatorX2apicIrrCallbackFn; 
        ia32eEmulatorX2apicIsrCallbackFn_t ia32eEmulatorX2apicIsrCallbackFn;  
        ia32eEmulatorX2apicEoiCallbackFn_t ia32eEmulatorX2apicEoiCallbackFn;
        ia32eEmulatorX2apicReadTmrCallbackFn_t ia32eEmulatorX2apicReadTmrCallbackFn;
    } x2apic;
} ia32eEmulatorCallbacks_t;
```

> **Note:** Any callbacks which are allowed to inject synchronous events such as advance, or exceptions are marked with '(synchronous)', failure to comply can cause unexpected behaviour.

Callbacks: <br>
```ia32eEmulatorInOutCallbackFn``` - (synchronous) Called during a PMIO access that is not permissible in its IOPB. <br>
```ia32eEmulatorRdmsrCallbackFn``` - (synchronous) Called on a rdmsr from an msr that isn't natively emulated. <br>
```ia32eEmulatorWrmsrCallbackFn``` - (synchronous) Called on a wrmsr to an msr that isn't natively emulated. <br>
```ia32eEmulatorEptFaultCallbackFn``` - (synchronous) Called when a vcpu accesses memory such that is not permissible in its EPTs. <br>
```ia32eEmulatorEptMisconfigCallbackFn``` - (synchronous) Called when a vcpus EPTs is configured incorrectly. <br>
```ia32eEmulatorRegsResetCallbackFn``` - Called whenever a guests registers are being reset due to an init ipi or power on. <br>

x2apic Callbacks: <br>
```ia32eEmulatorX2apicIrrCallbackFn``` - Called whenever the emulator sets a vector into the IRR with the vector (not including interrupts queued by a plugin), every interrupt corresponding to calls to this will be edge triggered.
```ia32eEmulatorX2apicIsrCallbackFn``` - Called whenever the emulator sets a vector into the ISR with the vector (including interrupts queued by a plugin) <br>
```ia32eEmulatorX2apicEoiCallbackFn``` - Called whenever an EOI is sent to the x2apic with the vector of the interrupt that was EOI'd <br>
```ia32eEmulatorX2apicReadTmrCallbackFn``` - Called with idx set to the tmr idx (0 - 8), expected to return true if the interrupt was level triggered otherwise false <br>

Return values: <br>
```IA32E_EMULATOR_CALLBACK_FAILURE``` - The event was unhandled <br> 
```IA32E_EMULATOR_CALLBACK_SUCCESS``` - The event was handled successfully <br>

## Callback APIs

```<ia32eEmulator.h>```

---

```c
bool ia32eEmulatorCallbackQueueEventSynthetic(bool advance, uint8_t vector, ia32eInterruptType_t type, 
                                              bool deliverErrcode, uint64_t errcode);
```

Queues an event to be carried out in the guest

---