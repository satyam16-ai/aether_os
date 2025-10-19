# Quick Start: Testing AetherOS Multitasking

This guide shows you how to quickly test the multitasking capabilities of AetherOS Phase 4.

---

## Prerequisites

- Linux system with:
  - `gcc` (or `i686-elf-gcc` for cross-compilation)
  - `nasm` (assembler)
  - `ld` (linker)
  - `qemu-system-i386` (x86 emulator)
  - `grub-mkrescue` (ISO creation)

---

## Build & Run

```bash
# Build the kernel
make clean
make

# Run in QEMU
qemu-system-i386 -cdrom build/aether.iso -m 32M
```

You should see the AetherOS boot screen with subsystem status showing TSS initialized.

---

## 5-Minute Multitasking Demo

### Step 1: Check System Status
```
cpu
mem
uptime
```

Verify the kernel is running properly.

### Step 2: Test Paging
```
paging status
paging test
```

View the virtual memory system (don't enable yet, just inspect).

### Step 3: Create Processes
```
ps list
```
You should see only the idle process (PID 0).

```
ps create
```
Creates two test processes: `test_proc_1` (PID 1) and `test_proc_2` (PID 2).

```
ps list
```
Now you should see 3 processes:
- PID 0: idle_process (RUNNING)
- PID 1: test_proc_1 (READY)
- PID 2: test_proc_2 (READY)

### Step 4: View Process Details
```
ps current
```
Shows the currently running process (idle_process).

```
ps info 1
```
Shows detailed information about test_proc_1.

### Step 5: Check Scheduler
```
sched stats
```

You should see:
- Status: DISABLED
- Ready Queue: 2 processes
- List of test_proc_1 and test_proc_2

### Step 6: Enable Multitasking! 🚀
```
sched start
```

**This is the moment!** The scheduler activates and processes start switching.

Wait a few seconds...

### Step 7: View Multitasking in Action
```
sched stats
```

You should now see:
- Status: ENABLED
- Current process alternates between processes
- Context Switches counter increasing
- Quantum remaining decreasing from 10 to 0
- Total runtime increasing for all processes

**The system is now multitasking!** Processes switch every 100ms (10 ticks).

### Step 8: Manual Context Switch
```
sched yield
```

This immediately switches to the next ready process. Try it multiple times and use `sched stats` to see the effect.

---

## Advanced: Stress Test (5+ Processes)

### Create Multiple Processes
```
ps create
ps create
ps create
```

This creates 6 test processes (2 per command).

```
ps list
```

You should see PIDs 1-6 all in READY state.

### Enable Multitasking
```
sched start
```

**Previously, this would cause a kernel panic with 5+ processes!**

Now with TSS support, it works perfectly. ✅

### Watch Them Run
```
sched stats
```

All processes share CPU time fairly. The scheduler rotates through them in round-robin fashion.

---

## Understanding the Test Processes

Both `test_proc_1` and `test_proc_2` execute the same behavior:

```c
void test_process_1(void) {
    for (int i = 0; i < 50; i++) {
        timer_sleep_ms(1000);  // Sleep 1 second
    }
    process_exit(0);  // Terminate after 50 seconds
}
```

- Each process runs for ~50 seconds
- Then terminates automatically
- State changes: READY → RUNNING → TERMINATED

After 50+ seconds, check:
```
ps list
```

You'll see some processes in TERMINATED state.

---

## What to Look For

### Success Indicators
✅ Kernel boots without errors  
✅ TSS shows as initialized  
✅ Processes can be created  
✅ Scheduler enables without crash  
✅ Context switches every 100ms  
✅ Statistics update correctly  
✅ 5+ processes run simultaneously  
✅ Manual yield works  
✅ No kernel panics  

### Common Issues

**"Process creation failed"**
- Likely hit the 256 process limit
- Use `ps list` to see current count

**"Scheduler already enabled"**
- Already running, use `sched stop` first

**Kernel panic on boot**
- Rebuild with `make clean && make`
- Check QEMU version (tested with 4.2+)

---

## Full Command Reference

### Paging Commands
| Command | Description |
|---------|-------------|
| `paging status` | Show page directory address |
| `paging test` | Display page table entries (first 10) |
| `paging enable` | Activate virtual memory |

### Process Commands
| Command | Description |
|---------|-------------|
| `ps` | List all processes |
| `ps list` | List all processes |
| `ps current` | Show current running process |
| `ps info <pid>` | Show details of process <pid> |
| `ps create` | Create 2 test processes |

### Scheduler Commands
| Command | Description |
|---------|-------------|
| `sched` | Show scheduler statistics |
| `sched stats` | Show scheduler statistics |
| `sched start` | Enable preemptive multitasking |
| `sched stop` | Disable scheduler |
| `sched yield` | Manually yield to next process |

### System Commands
| Command | Description |
|---------|-------------|
| `help` | Show all available commands |
| `clear` | Clear the screen |
| `cpu` | Show CPU information |
| `mem` | Show memory usage |
| `uptime` | Show system uptime |
| `echo <text>` | Print text to console |

---

## Technical Notes

### TSS (Task State Segment)
The TSS enables safe context switching from interrupt handlers:
- Provides kernel stack address per process
- Updated on every context switch: `tss_set_kernel_stack()`
- Required for privilege level transitions (ring 3 → ring 0)

### Context Switching
Happens every 100ms (10 timer ticks) when scheduler is enabled:
- Saves all registers (EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP, EIP, EFLAGS)
- Updates TSS with new process's kernel stack
- Restores registers from new process
- Jumps to new process's EIP

### Scheduler Algorithm
Round-robin with time quantum:
- Each process gets 10 ticks (100ms)
- When quantum expires, process moves to back of queue
- Next ready process gets CPU
- Fair CPU time distribution

---

## Troubleshooting

### Keyboard Not Responding
This was a bug we fixed. If it happens:
- Check that `pic_enable_irq(1)` is called in keyboard_init()
- Verify IRQ 1 is enabled with `irq status` (if you add that command)

### Kernel Panic with Multiple Processes
This was the major bug we fixed with TSS:
- Ensure `tss_init()` is called after `gdt_init()`
- Verify GDT has 6 entries (not 5)
- Check that `tss_set_kernel_stack()` is called before context switch

### Build Errors
```bash
# Clean build
make clean
make

# If still failing, check dependencies
which gcc
which nasm
which ld
which qemu-system-i386
```

---

## Next Steps

After verifying multitasking works:

1. **Read PHASE4_COMPLETE.md** - Technical deep dive
2. **Read TESTING.md** - Comprehensive test suite
3. **Experiment with commands** - Try different combinations
4. **Prepare for Phase 5** - User mode and system calls

---

## Success! 🎉

If you've successfully:
- Created multiple processes
- Enabled the scheduler
- Seen context switches happening
- Run 5+ processes simultaneously without crashes

**Congratulations!** You're running true preemptive multitasking on AetherOS!

The foundation for a complete operating system is now in place. Phase 4 is **COMPLETE**! ✓

---

*AetherOS - Built from scratch, one phase at a time*
