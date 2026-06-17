#!/usr/bin/env bash

set -e

KERNEL_ELF="$1"
OUTPUT_ISO="$2"

if [ -z "$KERNEL_ELF" ] || [ -z "$OUTPUT_ISO" ]; then
    echo "Usage: $0 <kernel.elf> <output.iso>"
    exit 1
fi

ISO_DIR="$(mktemp -d)"

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
GRUB_DIR="$SCRIPT_DIR/grub"

mkdir -p "$ISO_DIR/boot/grub"

cp "$KERNEL_ELF" "$ISO_DIR/boot/WorkhorseRT"
cp "$GRUB_DIR/grub.cfg" "$ISO_DIR/boot/grub/grub.cfg"

grub-mkrescue -o "$OUTPUT_ISO" "$ISO_DIR"

echo "ISO created at: $OUTPUT_ISO"
