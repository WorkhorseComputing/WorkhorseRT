#!/usr/bin/env bash

ISO="${1:-workhorse.iso}"
MEM="${2:-512M}"
CPUS="${3:-4}"

echo "  ISO:  $ISO"
echo "  MEM:  $MEM"
echo "  CPUS: $CPUS"

qemu-system-x86_64                  \
    -bios /usr/share/ovmf/OVMF.fd   \
    -cdrom "$ISO"                   \
    -serial stdio                   \
    -m "$MEM"                       \
    -smp "$CPUS"
