#!/usr/bin/env fish
# Test script to demonstrate exception handling in Aether OS
# Temporarily modifies kernel.c to trigger division by zero exception

echo "[+] Creating test version with division by zero exception..."

# Backup original kernel.c
cp src/kernel/kernel.c src/kernel/kernel.c.backup

# Enable division by zero test
sed -i 's|// volatile int test_div = 1 / 0; (void)test_div;|volatile int test_div = 1 / 0; (void)test_div;|' src/kernel/kernel.c

echo "[+] Building test kernel..."
make clean && make iso

echo "[+] Running test (should trigger panic screen)..."
echo "    Press Ctrl+C in QEMU window to exit after seeing panic"
make run

echo "[+] Restoring original kernel..."
mv src/kernel/kernel.c.backup src/kernel/kernel.c

echo "[+] Test complete!"