#!/usr/bin/env fish
# Run the built ISO in QEMU (i386)
if not test -f build/aether.iso
    echo "ISO not found. Building..."
    make iso
end
qemu-system-i386 -cdrom build/aether.iso $argv
