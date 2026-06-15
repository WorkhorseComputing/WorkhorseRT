# Domain

Domains are treated as virtual machines if the ```vm``` flag is set in the domains parameter. The pml4 of virtual machine domains are treated as their EPTs.

The first task initialised to a domain is the bsp.

_start must fit in 16 bits as the bsp will boot in realmode, with all segment selectors, and segment bases set to 0.