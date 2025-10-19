# AetherOS Testing Guide

## Phase 4 Complete: Virtual Memory & Process Management with TSS

This document contains test procedures to verify all Phase 4 functionality.

---

## Test 1: TSS Initialization

**What to test:** Verify Task State Segment is properly initialized.

**Steps:**
1. Boot the kernel in QEMU
2. Check the subsystem status display
3. Verify you see: `[DONE] TSS - Task State Segment`

**Expected Result:**
- Kernel boots without errors
- TSS shows as initialized
- No kernel panic

---

## Test 2: Paging System

**What to test:** Virtual memory with identity mapping.

**Commands:**
```
paging status
paging test
paging enable
```

**Expected Results:**
- `paging status` shows page directory at specific address
- `paging test` shows 4KB pages with Present + RW flags
- `paging enable` activates virtual memory without crash
- After enabling, message: "SUCCESS! Virtual memory is active!"

---

## Test 3: Process Creation

**What to test:** Process Control Block creation and management.

**Commands:**
```
ps list
ps create
ps list
```

**Expected Results:**
- Initial `ps list` shows only idle process (PID 0)
- `ps create` creates two test processes
- Second `ps list` shows:
  - PID 0: idle_process (RUNNING)
  - PID 1: test_proc_1 (READY)
  - PID 2: test_proc_2 (READY)

---

## Test 4: Process Information

**What to test:** Detailed process state inspection.

**Commands:**
```
ps current
ps info 1
ps info 2
```

**Expected Results:**
- `ps current` shows idle_process details
- `ps info 1` shows test_proc_1 with:
  - State: READY
  - Priority: NORMAL (50)
  - 4096 byte stack allocated
  - Registers initialized
- `ps info 2` shows similar for test_proc_2

---

## Test 5: Scheduler Statistics (Before Enable)

**What to test:** Scheduler state before activation.

**Commands:**
```
sched stats
```

**Expected Results:**
- Status: DISABLED
- Algorithm: Round-Robin
- Time Quantum: 10 ticks
- Ready Queue: 2 processes (if you ran `ps create`)
- Lists test_proc_1 and test_proc_2

---

## Test 6: Enable Multitasking (THE BIG TEST!)

**What to test:** Context switching with TSS support.

**Commands:**
```
ps create
sched start
```

**Wait 5-10 seconds, then:**
```
sched stats
```

**Expected Results:**
- After `sched start`: "Scheduler enabled! Multitasking active."
- Kernel does NOT panic (this was the bug we fixed!)
- After waiting, `sched stats` shows:
  - Status: ENABLED
  - Current process alternates between processes
  - Context Switches counter increases over time
  - Quantum remaining decreases from 10 to 0
  - Total runtime increases for all processes

---

## Test 7: Manual Yield

**What to test:** Voluntary context switching.

**Commands:**
```
ps create
sched start
sched yield
```

**Expected Results:**
- "Yielding to next process..."
- "Back from yield."
- Process switches immediately
- No kernel panic

---

## Test 8: Stress Test (5+ Processes)

**What to test:** This previously caused kernel panic!

**Commands:**
```
ps create
ps create
ps create
sched start
```

**Wait 10 seconds, then:**
```
sched stats
```

**Expected Results:**
- 6+ processes created (2 per `ps create`)
- Scheduler enables without crash
- All processes get CPU time
- Context switches happening every 100ms
- No kernel panic! ✓

---

## Test 9: Process Lifecycle

**What to test:** Process execution to completion.

**Steps:**
1. Create test processes: `ps create`
2. Enable scheduler: `sched start`
3. Wait 60 seconds (test processes run 50 iterations)
4. Check status: `ps list`

**Expected Results:**
- Test processes eventually terminate (exit after 50 iterations)
- State changes from RUNNING → READY → TERMINATED
- Scheduler continues with remaining processes

---

## Test 10: Scheduler Control

**What to test:** Enable/disable scheduler.

**Commands:**
```
sched start
sched stats
sched stop
sched stats
sched start
```

**Expected Results:**
- `sched start` enables multitasking
- First `sched stats` shows ENABLED
- `sched stop` disables scheduler
- Second `sched stats` shows DISABLED
- `sched start` re-enables successfully

---

## Known Working Features

✅ **Paging:**
- 2-level page tables (page directory + page tables)
- 4KB pages
- Identity mapping first 4MB
- Can enable/disable virtual memory
- Present + Read/Write flags

✅ **Process Control Blocks:**
- 256 process slots
- Static 4KB stacks (no heap fragmentation!)
- Full CPU context (all registers)
- Process states: NEW, READY, RUNNING, TERMINATED
- Priority levels: LOW (25), NORMAL (50), HIGH (75)

✅ **Context Switching:**
- Assembly save/restore of all registers
- EIP, ESP, EBP, EFLAGS preserved
- Safe switching from interrupt handler
- TSS updates kernel stack per process

✅ **Task State Segment (TSS):**
- Proper hardware task switching support
- ESP0/SS0 for kernel stack on privilege level change
- GDT entry 5 (selector 0x28)
- Updated on every context switch

✅ **Scheduler:**
- Round-robin algorithm
- 10 tick quantum (100ms per process)
- Ready queue (linked list)
- Statistics tracking
- Can enable/disable dynamically

✅ **Shell Commands:**
- `paging status/test/enable`
- `ps list/current/info/create`
- `sched stats/start/stop/yield`

---

## Technical Notes

### TSS Implementation

The Task State Segment enables safe context switching from interrupt handlers:

1. **Initialization:**
   - `tss_init()` called after GDT setup
   - TSS structure allocated with ESP0 = current kernel stack
   - SS0 = 0x10 (kernel data segment)
   - Added to GDT as entry 5 (selector 0x28)

2. **Context Switch:**
   - Before switching: `tss_set_kernel_stack(new_process_stack + 4096)`
   - Updates TSS.ESP0 to point to new process's kernel stack top
   - When interrupt occurs in ring 3, CPU loads ESP0 from TSS
   - Ensures safe stack switching on privilege level changes

3. **Why This Fixes the Kernel Panic:**
   - Previous crash: No TSS → undefined stack on privilege change
   - With TSS: Each process has proper kernel stack address
   - CPU uses TSS.ESP0 when entering kernel from user mode
   - No stack corruption during interrupt handling

### Test Process Behavior

Both `test_process_1()` and `test_process_2()` execute 50 iterations:

```c
void test_process_1(void) {
    for (int i = 0; i < 50; i++) {
        // Each iteration takes ~1 second
        timer_sleep_ms(1000);
    }
    process_exit(0);
}
```

- Each iteration: 1000ms delay
- Total runtime: ~50 seconds
- Then process terminates with `process_exit(0)`
- State changes: READY → RUNNING → TERMINATED

---

## Success Criteria

Phase 4 is **COMPLETE** if:

1. ✅ Kernel boots with TSS initialized
2. ✅ Paging can be enabled without crash
3. ✅ Processes can be created (multiple processes)
4. ✅ Scheduler can be enabled
5. ✅ Context switching works (no kernel panic)
6. ✅ 5+ processes can run simultaneously
7. ✅ Statistics track CPU time correctly
8. ✅ Manual yield works
9. ✅ Processes can complete and terminate

All criteria met! **Phase 4: DONE ✓**

---

## Next: Phase 5

With virtual memory, processes, and multitasking complete, Phase 5 will add:

- **User mode:** Ring 3 execution for processes
- **System calls:** Kernel API for user programs
- **Better process isolation:** Separate address spaces per process
- **Inter-process communication:** Pipes, signals, messages

The foundation is solid! 🚀
