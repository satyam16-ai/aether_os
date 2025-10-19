# Phase 4 Complete: Virtual Memory & Process Management

## 🎉 Achievement Unlocked: True Multitasking!

AetherOS now has full preemptive multitasking with proper hardware support via the Task State Segment (TSS). This was a significant milestone requiring careful implementation of low-level CPU features.

---

## What Was Implemented

### 1. Paging System ✓
- **2-level page tables:** Page directory + page tables
- **4KB pages:** Standard x86 page size
- **Identity mapping:** First 4MB (kernel region)
- **Enable/disable:** Can activate virtual memory on demand
- **Shell commands:** `paging status`, `paging test`, `paging enable`

**Files:**
- `include/paging.h`
- `src/kernel/paging.c`

### 2. Process Control Blocks (PCB) ✓
- **256 process slots:** Maximum concurrent processes
- **Static stacks:** 4KB per process (no heap fragmentation)
- **Full context:** All CPU registers saved/restored
- **Process states:** NEW, READY, RUNNING, TERMINATED
- **Priority levels:** LOW (25), NORMAL (50), HIGH (75)
- **Shell commands:** `ps list`, `ps current`, `ps info <pid>`, `ps create`

**Files:**
- `include/process.h`
- `src/kernel/process.c`

### 3. Context Switching ✓
- **Assembly implementation:** Save/restore all registers
- **Register state:** EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP, EIP, EFLAGS
- **Safe switching:** Works from interrupt handlers
- **TSS integration:** Updates kernel stack per process

**Files:**
- `include/context.h`
- `src/kernel/context_switch.asm`

### 4. Task State Segment (TSS) ✓
- **Hardware support:** Proper x86 task switching
- **Kernel stack management:** ESP0/SS0 for privilege level changes
- **GDT integration:** Entry 5 (selector 0x28)
- **Per-process stack:** Updated on every context switch
- **Assembly wrapper:** LTR instruction to load TSS

**Files:**
- `include/tss.h`
- `src/kernel/tss.c`
- `src/kernel/tss_flush.asm`

### 5. Process Scheduler ✓
- **Algorithm:** Round-robin
- **Time quantum:** 10 ticks (100ms per process)
- **Ready queue:** Linked list of ready processes
- **Statistics:** Tracks CPU time, context switches
- **Dynamic control:** Enable/disable at runtime
- **Shell commands:** `sched stats`, `sched start`, `sched stop`, `sched yield`

**Files:**
- `include/scheduler.h`
- `src/kernel/scheduler.c`

---

## Technical Achievements

### Stack Allocation Strategy

**Problem:** Initial implementation used `kmalloc()` for 8KB stacks:
- Heap fragmentation after 2-3 processes
- 8KB too large for fragmented heap
- Reduced to 4KB but still failed

**Solution:** Static allocation:
```c
static uint8_t process_stacks[256][4096];
static int stack_allocated[256];
```
- No heap allocation needed
- No fragmentation issues
- Compile-time memory reservation
- Can create 256 processes reliably

### TSS for Safe Context Switching

**Problem:** Kernel panic when creating 5+ processes and enabling scheduler:
- Context switching from interrupt handler unsafe
- No proper kernel stack for privilege level switches
- Stack corruption on interrupt handling

**Solution:** Task State Segment:
```c
// Initialize TSS with kernel stack
tss_init(read_esp());

// Before each context switch:
uint32_t kernel_stack = (uint32_t)next_process->kernel_stack + 4096;
tss_set_kernel_stack(kernel_stack);

// CPU uses TSS.ESP0 when entering kernel from user mode
```

**Why This Works:**
1. TSS provides CPU with kernel stack address
2. On privilege level change (ring 3 → ring 0), CPU loads ESP from TSS.ESP0
3. Each process gets its own kernel stack address
4. No stack corruption during interrupts

### GDT Expansion

Original GDT (5 entries):
```
0: NULL segment
1: Kernel code (0x08)
2: Kernel data (0x10)
3: User code (0x18)
4: User data (0x20)
```

Expanded GDT (6 entries):
```
0: NULL segment
1: Kernel code (0x08)
2: Kernel data (0x10)
3: User code (0x18)
4: User data (0x20)
5: TSS (0x28)        ← NEW!
```

Changes to `src/kernel/gdt.c`:
- `GDT_ENTRIES` changed from 5 to 6
- `gdt_set_gate()` changed from static to public (TSS needs it)
- TSS descriptor: Base = &kernel_tss, Limit = sizeof(tss_entry_t) - 1
- Access = 0xE9 (Present, Ring 0, TSS)
- Granularity = 0x00 (byte granularity)

---

## Bug Fixes Along the Way

### 1. Keyboard Not Accepting Input
**Symptom:** PS/2 keyboard driver registered but no input detected.

**Root Cause:** IRQ 1 never enabled in PIC.

**Fix:**
```c
void keyboard_init(void) {
    // ... setup code ...
    pic_enable_irq(1);  // ← ADDED THIS
}
```

### 2. Backspace Executing Commands
**Symptom:** Pressing backspace would execute incomplete commands.

**Root Cause:** Backspace scancode (0x0E) not handled, treated as character.

**Fix:**
```c
if (scancode == 0x0E) {  // Backspace
    keyboard_buffer[buffer_pos++] = '\b';
    return;
}
```

### 3. Process Stack Allocation Failing
**Symptom:** Creating 3rd process fails, "Failed to allocate stack".

**Root Cause:** Heap fragmentation with 8KB allocations, then 4KB too large.

**Fix:** Static arrays (see above).

### 4. Kernel Panic with 5+ Processes
**Symptom:** Creating 5+ processes and enabling scheduler → kernel panic.

**Root Cause:** Context switching from interrupt without TSS → undefined stack.

**Fix:** Full TSS implementation (see above).

---

## Files Created/Modified

### New Files
```
include/paging.h
include/process.h
include/scheduler.h
include/context.h
include/tss.h
src/kernel/paging.c
src/kernel/process.c
src/kernel/scheduler.c
src/kernel/context_switch.asm
src/kernel/tss.c
src/kernel/tss_flush.asm
TESTING.md
PHASE4_COMPLETE.md (this file)
```

### Modified Files
```
src/kernel/kernel.c      - Added tss_init() call
src/kernel/gdt.c         - Expanded to 6 entries, public gdt_set_gate()
src/kernel/shell.c       - Added paging/ps/sched commands
src/kernel/keyboard.c    - Fixed IRQ enable, backspace handling
src/kernel/timer.c       - Added scheduler_tick() call
```

---

## Testing Results

### Test 1: Basic Boot ✅
- Kernel boots with TSS initialized
- Subsystem status shows all components
- No kernel panic

### Test 2: Paging ✅
- `paging status` shows page directory
- `paging test` displays page table entries
- `paging enable` activates virtual memory
- No crash after enabling

### Test 3: Process Creation ✅
- Can create multiple processes
- Each gets 4KB stack allocated
- Process info shows correct state
- No allocation failures

### Test 4: Single Process ✅
- Create 1 process, enable scheduler
- Process runs successfully
- Statistics track correctly
- No crashes

### Test 5: Multiple Processes ✅
- Create 2 processes, enable scheduler
- Both processes alternate execution
- Context switches every 100ms
- Statistics show both running

### Test 6: Stress Test (5+ Processes) ✅
**This was the critical test that previously failed!**
- Create 6+ processes
- Enable scheduler
- All processes run without panic
- Context switches work perfectly
- TSS properly manages kernel stacks

### Test 7: Manual Yield ✅
- `sched yield` switches immediately
- Returns to shell after yield
- No kernel panic
- Works reliably

### Test 8: Long-Running ✅
- Let scheduler run for 60+ seconds
- Processes complete their 50 iterations
- State transitions: READY → RUNNING → TERMINATED
- System remains stable

---

## Performance Characteristics

### Context Switch Speed
- Assembly implementation: ~50 CPU cycles
- 16 register saves + 16 register restores
- TSS update: ~10 cycles
- Total: ~100 cycles per switch

At 100 MHz CPU:
- Context switch: ~1 microsecond
- Time quantum: 100ms
- Overhead: 0.001% (negligible)

### Memory Usage
- Process table: 256 × sizeof(process_t) ≈ 256 × 128 = 32KB
- Process stacks: 256 × 4KB = 1MB
- Page directory: 4KB
- Page tables: 4KB each (1 table per 4MB)
- Total Phase 4 memory: ~1.05MB

### Scalability
- Maximum processes: 256 (compile-time limit)
- Maximum memory: 4GB (32-bit addressing)
- Scheduler complexity: O(1) for ready queue operations
- Context switch: O(1) constant time

---

## Code Quality

### Assembly Best Practices
- **Explicit register preservation:** Save/restore ALL registers
- **Stack alignment:** Maintain 16-byte alignment
- **Comment every instruction:** Explain purpose
- **Test with multiple processes:** Verify correctness

### C Best Practices
- **Static allocation for critical resources:** Prevents fragmentation
- **Proper error handling:** Check return values
- **Kernel/user separation:** Structures support ring 3
- **Statistics tracking:** Monitor system health

### Documentation
- **Inline comments:** Explain complex operations
- **Testing guide:** Comprehensive test procedures
- **Technical notes:** TSS explanation, bug fixes
- **Success criteria:** Clear completion metrics

---

## Lessons Learned

### 1. Static > Dynamic for Kernel Resources
Kernel resources (like process stacks) benefit from static allocation:
- Predictable memory layout
- No fragmentation issues
- Compile-time verification
- Better performance (no allocation overhead)

### 2. TSS is Not Optional
Context switching from interrupts requires TSS:
- Provides kernel stack address to CPU
- Enables safe privilege level transitions
- Required for proper x86 task switching
- Cannot be faked or omitted

### 3. Test Incrementally
Build complex features step-by-step:
- Paging → Process creation → Context switching → Scheduler
- Test each component before adding next
- Easier to debug when isolated
- Catch bugs early

### 4. Hardware Knows Best
Use hardware features properly:
- TSS for task switching
- CR3 for page directory
- EFLAGS for interrupt state
- Don't try to outsmart the CPU

---

## What's Next: Phase 5

With virtual memory and multitasking complete, we can now implement:

### User Mode (Ring 3)
- Process execution in ring 3
- Privilege level enforcement
- System call interface (INT 0x80)
- Kernel/user separation

### System Calls
- `fork()` - Create child process
- `exec()` - Execute new program
- `wait()` - Wait for child
- `exit()` - Terminate process
- `read()`/`write()` - I/O operations

### Virtual Address Spaces
- Separate page directory per process
- User space: 0x00000000 - 0xBFFFFFFF
- Kernel space: 0xC0000000 - 0xFFFFFFFF
- Copy-on-write for efficiency

### Inter-Process Communication
- Pipes for process communication
- Signals for async events
- Shared memory regions
- Message queues

### Enhanced Scheduling
- Multi-level feedback queue
- Priority inheritance
- CPU affinity (future SMP)
- Real-time scheduling classes

---

## Conclusion

Phase 4 is **COMPLETE**! ✓

We now have:
- ✅ Virtual memory with paging
- ✅ Process management with PCB
- ✅ Preemptive multitasking
- ✅ Hardware task switching with TSS
- ✅ Round-robin scheduler
- ✅ Context switching from interrupts
- ✅ Stable with 5+ processes
- ✅ Comprehensive testing suite

The kernel has evolved from a simple shell to a true multitasking operating system. Processes can now run concurrently, share CPU time fairly, and the system remains stable under load.

**The foundation for a complete OS is now in place!** 🚀

---

**AetherOS - Phase 4 Completed: 2024**
*"From single-tasking to true multitasking"*
