# Multiboot2

Multiboot2 information is stored in ia32eGlobal which can be retrieved by calling ```ia32eGlobal_t *ia32eGetGlobalPtr(void)```.

Plugins may parse the Multiboot2 structure to retrieve system information such as the memory map and any other tags required during domain setup.