# Plugins

Plugins are routines which run after architecture specific initialization and before kernel handoff. 

Plugins are responsible for initializing domains, and tasks. This includes setting up domains memory layout, setting up MMIO regions, loading programs into the address space of domains, and performing any platform initialization not already handled by architecture specific initialization. 

Plugins can call routines defined by architecture specific code only if stated safe to do so. Developers should consult the documentation associated with the architecture they are targeting, to understand best practises, security considerations, and do's and don'ts.

Plugins are currently required to be written in C (support for additional languages may be introduced in the future).

Plugins must be placed in the src/plugin directory to be compiled with the kernel. 

- [kPlugin API](kPlugin.md)