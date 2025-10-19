# Phase 5: User Mode & System Calls

## Overview

With Phase 4 complete (virtual memory, processes, TSS, multitasking), we now need to add **user mode execution** and **system calls** to properly isolate user programs from the kernel.

---

## Goals

### Primary Objectives
1. **User Mode (Ring 3)** - Run processes in unprivileged mode
2. **System Call Interface** - Kernel API for user programs (INT 0x80)
3. **Process Lifecycle** - fork(), exec(), wait(), exit()
4. **Separate Address Spaces** - Per-process virtual memory
5. **Basic IPC** - Pipes or signals

### Why This Matters
- **Security:** User programs can't crash the kernel
- **Stability:** Bugs in user programs won't affect system
- **Isolation:** Processes can't interfere with each other
- **Standard API:** Programs use syscalls like UNIX/Linux

---

## Step 1: User Mode Execution

### What to Implement

#### 1.1 Ring 3 Stack Setup
```c
// In process.c
void process_setup_user_mode(process_t* process) {
    // Allocate user stack (separate from kernel stack)
    uint32_t user_stack = 0x00800000;  // User space
    
    // Set up stack frame for iret to ring 3
    uint32_t* stack = (uint32_t*)(process->kernel_stack + 4096);
    *(--stack) = 0x23;           // SS (user data selector + RPL 3)
    *(--stack) = user_stack;     // ESP
    *(--stack) = 0x202;          // EFLAGS (IF enabled)
    *(--stack) = 0x1B;           // CS (user code selector + RPL 3)
    *(--stack) = (uint32_t)entry_point;  // EIP
}
```

#### 1.2 Enter User Mode
```asm
; In user_mode_enter.asm
global enter_user_mode
enter_user_mode:
    mov ax, 0x23        ; User data segment (GDT entry 4 | RPL 3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Stack frame set up by process_setup_user_mode()
    iret                ; Pop CS, EIP, EFLAGS, SS, ESP
```

#### 1.3 Test User Program
```c
// Simple user mode test
void user_test_program(void) {
    // This will run in ring 3
    while (1) {
        // Try to do privileged operation -> GPF!
        // Use syscall instead
    }
}
```

### Files to Create
- `include/usermode.h`
- `src/kernel/usermode.c`
- `src/kernel/user_mode_enter.asm`

### Testing
1. Create a process in user mode
2. Verify CPL = 3 (check CS register)
3. Try privileged instruction → should cause #GP (General Protection Fault)

---

## Step 2: System Call Interface

### What to Implement

#### 2.1 System Call Handler
```c
// In syscall.c
#define SYSCALL_EXIT   1
#define SYSCALL_WRITE  2
#define SYSCALL_READ   3
#define SYSCALL_FORK   4
#define SYSCALL_EXEC   5

void syscall_handler(registers_t* regs) {
    uint32_t syscall_num = regs->eax;
    uint32_t arg1 = regs->ebx;
    uint32_t arg2 = regs->ecx;
    uint32_t arg3 = regs->edx;
    
    uint32_t result = 0;
    
    switch (syscall_num) {
        case SYSCALL_EXIT:
            sys_exit(arg1);
            break;
        case SYSCALL_WRITE:
            result = sys_write(arg1, (char*)arg2, arg3);
            break;
        // ... more syscalls
    }
    
    regs->eax = result;  // Return value
}
```

#### 2.2 Register Syscall Interrupt
```c
// In kernel.c
idt_set_gate(0x80, (uint32_t)syscall_wrapper, 0x08, 0xEE);
// 0xEE = Present + DPL 3 (callable from user mode)
```

#### 2.3 Syscall Wrapper Assembly
```asm
; In syscall_handler.asm
extern syscall_handler

global syscall_wrapper
syscall_wrapper:
    ; Save all registers
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    push ds
    push es
    push fs
    push gs
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    
    ; Call C handler
    push esp            ; Pass register struct
    call syscall_handler
    add esp, 4
    
    ; Restore registers
    pop gs
    pop fs
    pop es
    pop ds
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    
    iret
```

#### 2.4 User-Space Syscall Library
```c
// In userlib.h (for user programs)
static inline int syscall_exit(int status) {
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(1), "b"(status));
    return ret;
}

static inline int syscall_write(int fd, const char* buf, size_t len) {
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(2), "b"(fd), "c"(buf), "d"(len));
    return ret;
}
```

### Files to Create
- `include/syscall.h`
- `src/kernel/syscall.c`
- `src/kernel/syscall_handler.asm`
- `include/userlib.h` (for user programs)

### Testing
1. Create user program that calls `syscall_write()`
2. Verify output appears on screen
3. Test `syscall_exit()` - process should terminate

---

## Step 3: Process Lifecycle (fork/exec)

### What to Implement

#### 3.1 fork() - Clone Current Process
```c
pid_t sys_fork(void) {
    process_t* parent = current_process;
    process_t* child = process_create(parent->name, NULL, parent->priority);
    
    if (!child) return -1;
    
    // Copy parent's address space
    paging_clone_directory(child, parent);
    
    // Copy parent's registers
    memcpy(&child->registers, &parent->registers, sizeof(registers_t));
    
    // Set return values
    parent->registers.eax = child->pid;  // Parent gets child PID
    child->registers.eax = 0;            // Child gets 0
    
    // Set parent-child relationship
    child->parent = parent;
    
    scheduler_add_process(child);
    return child->pid;
}
```

#### 3.2 exec() - Replace Process Image
```c
int sys_exec(const char* path, char** argv) {
    // 1. Load new program from filesystem (Phase 6)
    // 2. Clear current address space
    // 3. Map new program into memory
    // 4. Set up new stack with arguments
    // 5. Jump to entry point
    
    // For now: simple implementation
    process_t* proc = current_process;
    
    // Reset process state
    memset(&proc->registers, 0, sizeof(registers_t));
    
    // Set up entry point
    proc->registers.eip = (uint32_t)new_program_entry;
    proc->registers.esp = 0x00800000;  // User stack
    
    return 0;
}
```

#### 3.3 wait() - Wait for Child
```c
pid_t sys_wait(int* status) {
    // Wait for any child to exit
    while (1) {
        // Check if any child has exited
        for (int i = 0; i < 256; i++) {
            if (process_table[i].state == PROCESS_STATE_TERMINATED &&
                process_table[i].parent == current_process) {
                
                pid_t child_pid = process_table[i].pid;
                if (status) {
                    *status = process_table[i].exit_code;
                }
                
                // Clean up child
                process_table[i].state = PROCESS_STATE_FREE;
                return child_pid;
            }
        }
        
        // No terminated children, yield CPU
        scheduler_yield();
    }
}
```

#### 3.4 exit() - Terminate Process
```c
void sys_exit(int status) {
    process_t* proc = current_process;
    
    proc->state = PROCESS_STATE_TERMINATED;
    proc->exit_code = status;
    
    // Wake up parent if waiting
    if (proc->parent && proc->parent->state == PROCESS_STATE_WAITING) {
        scheduler_add_process(proc->parent);
    }
    
    // Orphan children - set parent to init (PID 1)
    for (int i = 0; i < 256; i++) {
        if (process_table[i].parent == proc) {
            process_table[i].parent = &process_table[1];
        }
    }
    
    // Never returns - scheduler will switch to another process
    scheduler_yield();
    for (;;);  // Just in case
}
```

### Files to Modify
- `src/kernel/process.c` - Add fork/exec/wait/exit
- `src/kernel/syscall.c` - Add syscall numbers
- `include/process.h` - Add function prototypes

### Testing
1. Test fork() - child should be clone of parent
2. Test exec() - process should transform into new program
3. Test wait() - parent should wait for child
4. Test exit() - process should terminate cleanly

---

## Step 4: Separate Address Spaces

### What to Implement

#### 4.1 Per-Process Page Directories
```c
// In paging.c
page_directory_t* paging_create_directory(void) {
    page_directory_t* dir = (page_directory_t*)kmalloc_aligned(4096);
    memset(dir, 0, 4096);
    
    // Identity map kernel (0xC0000000 - 0xFFFFFFFF)
    for (uint32_t i = 768; i < 1024; i++) {
        dir->entries[i] = kernel_directory->entries[i];
    }
    
    return dir;
}

void paging_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    asm volatile("mov %0, %%cr3" :: "r"(dir));
}
```

#### 4.2 Copy-on-Write for fork()
```c
// Clone parent's address space (copy-on-write)
void paging_clone_directory(process_t* child, process_t* parent) {
    child->page_directory = paging_create_directory();
    
    // For each user page table (0-767)
    for (uint32_t i = 0; i < 768; i++) {
        if (parent->page_directory->entries[i] & PAGE_PRESENT) {
            // Mark both parent and child pages as read-only
            // On write -> page fault -> copy page
            parent->page_directory->entries[i] &= ~PAGE_WRITE;
            child->page_directory->entries[i] = parent->page_directory->entries[i];
        }
    }
}
```

#### 4.3 Page Fault Handler for CoW
```c
void page_fault_handler(registers_t* regs) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));
    
    int present = regs->err_code & 0x1;
    int write = regs->err_code & 0x2;
    
    if (!present) {
        // Page not present - allocate
        paging_allocate_page(faulting_address);
    } else if (write) {
        // Copy-on-write
        paging_copy_page(faulting_address);
    } else {
        // Real page fault - terminate process
        printk("Page fault at 0x%x in process %d\n", 
               faulting_address, current_process->pid);
        process_exit(-1);
    }
}
```

### Files to Modify
- `src/kernel/paging.c` - Add per-process directories
- `src/kernel/process.c` - Integrate with fork/exec
- `include/paging.h` - Add new functions

### Testing
1. Create two processes
2. Verify they have separate address spaces
3. Test CoW - write to shared page should copy
4. Verify processes can't access each other's memory

---

## Step 5: Basic IPC (Inter-Process Communication)

### Option A: Pipes (Simpler)

```c
// In pipe.c
#define PIPE_SIZE 4096

typedef struct pipe {
    char buffer[PIPE_SIZE];
    uint32_t read_pos;
    uint32_t write_pos;
    uint32_t count;
    process_t* reader;
    process_t* writer;
} pipe_t;

int sys_pipe(int pipefd[2]) {
    pipe_t* pipe = kmalloc(sizeof(pipe_t));
    memset(pipe, 0, sizeof(pipe_t));
    
    // Allocate file descriptors
    pipefd[0] = fd_allocate(current_process, pipe, FD_READ);
    pipefd[1] = fd_allocate(current_process, pipe, FD_WRITE);
    
    return 0;
}

ssize_t pipe_read(pipe_t* pipe, char* buf, size_t count) {
    while (pipe->count == 0) {
        scheduler_yield();  // Wait for data
    }
    
    size_t to_read = (count < pipe->count) ? count : pipe->count;
    for (size_t i = 0; i < to_read; i++) {
        buf[i] = pipe->buffer[pipe->read_pos];
        pipe->read_pos = (pipe->read_pos + 1) % PIPE_SIZE;
    }
    pipe->count -= to_read;
    
    return to_read;
}
```

### Option B: Signals (More Complex)

```c
// In signal.c
#define SIGKILL  9
#define SIGTERM  15
#define SIGUSR1  10

void signal_send(process_t* target, int signal) {
    target->pending_signals |= (1 << signal);
    
    if (target->state == PROCESS_STATE_WAITING) {
        scheduler_add_process(target);
    }
}

void signal_handle(void) {
    process_t* proc = current_process;
    
    for (int sig = 0; sig < 32; sig++) {
        if (proc->pending_signals & (1 << sig)) {
            proc->pending_signals &= ~(1 << sig);
            
            if (proc->signal_handlers[sig]) {
                proc->signal_handlers[sig](sig);
            } else {
                // Default action
                if (sig == SIGKILL || sig == SIGTERM) {
                    process_exit(-1);
                }
            }
        }
    }
}
```

### Files to Create
- `include/pipe.h` or `include/signal.h`
- `src/kernel/pipe.c` or `src/kernel/signal.c`

### Testing
**Pipes:**
1. Create pipe with `pipe()`
2. Fork process
3. Parent writes, child reads
4. Verify data transfers correctly

**Signals:**
1. Send signal to process
2. Verify handler is called
3. Test SIGKILL terminates process

---

## Implementation Order

### Week 1: User Mode
- [ ] Create user mode stack setup
- [ ] Implement enter_user_mode()
- [ ] Test: Process runs in ring 3
- [ ] Test: Privileged instruction causes GPF

### Week 2: System Calls
- [ ] Create syscall handler (INT 0x80)
- [ ] Implement sys_exit()
- [ ] Implement sys_write()
- [ ] Test: User program calls syscalls

### Week 3: Process Lifecycle
- [ ] Implement sys_fork()
- [ ] Implement sys_exec() (basic version)
- [ ] Implement sys_wait()
- [ ] Test: Fork/exec/wait workflow

### Week 4: Virtual Memory
- [ ] Per-process page directories
- [ ] Page directory switching
- [ ] Copy-on-write for fork()
- [ ] Test: Isolated address spaces

### Week 5: IPC
- [ ] Implement pipes OR signals
- [ ] Test parent-child communication
- [ ] Verify bidirectional data flow

---

## Success Criteria

Phase 5 is **COMPLETE** when:

1. ✅ Processes run in user mode (ring 3)
2. ✅ System call interface works (INT 0x80)
3. ✅ fork() creates child processes
4. ✅ exec() replaces process image
5. ✅ wait() blocks until child exits
6. ✅ exit() terminates process cleanly
7. ✅ Each process has separate address space
8. ✅ Processes can communicate (pipes or signals)
9. ✅ No kernel panics
10. ✅ Shell can launch user programs

---

## Testing Workflow

### Full Integration Test
```c
// User program: test.c
#include <userlib.h>

int main(int argc, char** argv) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        write(1, "Hello from child!\n", 18);
        exit(0);
    } else {
        // Parent process
        write(1, "Hello from parent!\n", 19);
        int status;
        wait(&status);
        write(1, "Child exited\n", 13);
    }
    
    return 0;
}
```

### Expected Output
```
Hello from parent!
Hello from child!
Child exited
```

---

## Dependencies

### Required Before Starting
- ✅ Phase 4 complete (paging, processes, TSS, multitasking)
- ✅ Stable context switching
- ✅ Process management working

### Nice to Have
- File system (for exec() to load programs)
- ELF loader (to parse executables)
- Standard library (libc for user programs)

### Can Be Added Later
- Advanced IPC (shared memory, message queues)
- POSIX compatibility
- Multi-threading
- SMP support

---

## Resources

### Documentation
- Intel Manual Vol 3: System Programming Guide
- OSDev Wiki: User Mode, System Calls
- Linux source: kernel/fork.c, kernel/exec.c

### Similar Projects
- JamesM's Kernel Tutorial (user mode)
- ToaruOS (full syscall interface)
- xv6 (educational UNIX clone)

---

## Phase 6 Preview

After Phase 5, we'll add:
- **File System** - VFS layer with ext2/FAT support
- **Device Drivers** - Block devices, character devices
- **Program Loader** - ELF binary parsing
- **Standard Library** - Basic libc for user programs
- **Shell Improvements** - Run executables, pipes, redirects

---

**Phase 5 will transform AetherOS from a kernel with multitasking into a true operating system with user programs!** 🚀
