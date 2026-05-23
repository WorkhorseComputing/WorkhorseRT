# Porting

WorkhorseRT is designed to be portable across architectures. All hardware specific logic is isolated via a porting interface. To support a new architecture, an implementation must register the required interfaces and hand control to routines implemented by the kernel core.

# Export

These are the architecture specific services that the kernel core relies on. A port must implement all of them unless explicitly stated to be optional.

- [Export](export/)

# Import

These are callbacks that architecture specific code must invoke when hardware events occur.

- [Import](import/)

# Data structures

The following data structures must be defined by architecture specific code in a header which is included in <defs.h> based on the kernel configuration.

```c
typedef struct archSchedCtx
{
    ...
} archSchedCtx_t;

typedef struct archSchedThreadParam
{
    ...
} archSchedThreadParam_t;

typedef struct archSchedThreadInfo
{
    ...
} archSchedThreadInfo_t;

typedef struct archSchedLsrParam
{
    ...
} archSchedLsrParam_t;

typedef struct archSchedLsrInfo
{
    ...
} archSchedLsrInfo_t;

typedef struct archDomainInfo
{
    ...
} archDomainInfo_t;

typedef struct archDomainParam
{
    ...
} archDomainParam_t;
```

# Linker

The following linker sections must be defined by architecture specific code:

- ```.kInit```
- ```__kInitStart``` 
- ```__kInitEnd```
- ```.kPlugin```
- ```__kPluginStart```   
- ```__kPluginEnd```

# Kconfig

```config CMAKELISTS_SUBDIR``` must be defined in architecture specific Kconfig, with a path to the directory containing the relevant CMakeLists.txt file.