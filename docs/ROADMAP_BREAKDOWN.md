# Aether OS – Detailed Roadmap & Milestone Breakdown

Status Legend: 🔴 Not Started · 🟡 In Progress · 🟢 Complete · 🔁 Iterative · ⚙️ Planned Refactor

---
## Phase 0 – Environment & Tooling (You Are Here)
Goal: Reliable, repeatable build + run loop (boot -> kernel stub) in <10s.

Milestones
1. Toolchain & Utilities
   - [ ] nasm installed
   - [ ] grub-mkrescue + xorriso + mtools installed
   - [ ] qemu-system-i386 installed
   - [ ] build-essential (gcc, ld, make) present
   - [ ] Optional cross compiler (i686-elf-gcc) built or installed
2. Repository Hygiene
   - [x] Makefile + linker script
   - [x] Enhanced kernel with printk facility
   - [x] Multiboot header in place
   - [x] Freestanding headers (stdint.h, stddef.h, stdarg.h)
   - [ ] CI stub (GitHub Actions) for build
   - [ ] Coding style guidelines doc
3. Developer Experience
   - [ ] Script: run headless QEMU test mode
   - [ ] Script: symbol map dump (nm/objdump)
   - [ ] Script: clean rebuild + run (fast loop)

Acceptance Criteria
- `make iso && make run` boots to VGA text banner without errors.
- Fresh clone + dependency install reproduces build.

---
## Phase 1 – Early Kernel Foundations
Goal: Transition scaffolding to structured kernel initialization order.

Milestones
1. CPU & Memory Setup
   - [x] Global Descriptor Table (GDT) + flush (flat memory model)
   - [x] Interrupt Descriptor Table (IDT) with stubs for CPU exceptions (0–31)
   - [x] Basic ISR handler with detailed fault info & register dumps
   - [x] Panic handler with stack traces and colored output
   - [ ] Programmable Interrupt Controller (PIC) remap
   - [ ] PIT timer init + global tick counter
2. Memory Management
   - [ ] Parse Multiboot memory map
   - [ ] Physical frame allocator (bitmap)
   - [ ] Basic paging enable (identity map + kernel space)
   - [ ] Higher-half kernel design doc (defer implementation)
   - [ ] Simple bump allocator (early heap)
3. Console & Diagnostics
   - [x] Full printk with format subsets (%s %x %u %d %c %p %%)
   - [x] Color-coded log levels (INFO/WARN/ERR)
   - [x] Scrolling console support
   - [x] Comprehensive panic handler with register dumps, stack traces, page fault analysis
4. Build/Debug Tooling
   - [ ] Debug symbol verification (objdump script)
   - [ ] QEMU gdb stub launch mode

Acceptance Criteria
- [✅] GDT loads successfully with kernel/user code/data segments
- [✅] IDT loads with all 32 exception handlers
- [✅] Intentional divide-by-zero triggers detailed panic screen with registers
- [ ] Timer increments tick counter (visible via printk)
- [ ] Allocation of sequential physical frames returns monotonic addresses

---
## Phase 2 – Basic Device & Input Layer
Goal: Interact with user & time, lay scaffolding for drivers.

Milestones
1. Keyboard
   - [ ] PS/2 controller basic init
   - [ ] Scancode to ASCII (US layout)
   - [ ] Simple line buffer
   - [ ] Input echo to console
2. Enhanced Console
   - [ ] Scrolling support (shift video memory)
   - [ ] Clear screen command
   - [ ] Compile-time selectable VGA vs future framebuffer
3. Time & Scheduling Prep
   - [ ] Calibrate PIT / HPET (choose basis)
   - [ ] Sleep (busy) + later tick wait abstraction
4. Randomness (Entropy Seeding Placeholder)
   - [ ] Seed structure design (not a real RNG yet)

Acceptance Criteria
- User can type and see echoed characters.
- Console scrolls past 25 lines without corruption.

---
## Phase 3 – Process & Syscall Scaffold
Goal: Minimal task switching & API boundary for future AI hooks.

Milestones
1. Context & Task Model
   - [ ] Task control block structure (register set, state)
   - [ ] Manual creation of 2 dummy tasks
   - [ ] Cooperative switch (timer-disabled)
   - [ ] Preemptive switch (timer ISR integration)
2. Syscall Interface (Stub)
   - [ ] Syscall dispatch table
   - [ ] syscall(int num, ...) inline asm stub
   - [ ] Initial syscalls: write_console, get_ticks, yield
3. Scheduler Policy (v0)
   - [ ] Round-robin w/ fixed quantum
   - [ ] Statistics counters (switches, runtime ticks)
4. Diagnostics
   - [ ] Task listing (debug command)

Acceptance Criteria
- Two tasks alternately print IDs without deadlock.
- Syscall path validated (user-mode TBD future; currently kernel-mode simulation).

---
## Phase 4 – Virtual Memory & Higher-Half Transition
Goal: Memory isolation scaffolding for later user-space.

Milestones
1. Higher-Half Mapping
   - [ ] Paging structures for kernel at 0xC0000000 (example) or chosen base
   - [ ] Dual mapping (low + high) during transition
   - [ ] Update linker + addresses
2. Page Fault Handling
   - [ ] Basic fault decoder (present, rw, us, reserved)
   - [ ] Panic on unexpected fault
3. Kernel Heap v1
   - [ ] Free-list allocator
   - [ ] Fragmentation metrics counters

Acceptance Criteria
- Kernel runs identically from higher-half virtual base.
- Intentional invalid pointer triggers informative page fault message.

---
## Phase 5 – Filesystem & Init Abstractions
Goal: Structured resource access.

Milestones
1. VFS Layer
   - [ ] vnode/inode abstraction design
   - [ ] RAMFS implementation (static files baked at build)
   - [ ] Basic open/read interface
2. Loader Concepts (Planning Only)
   - [ ] ELF parsing draft doc
3. Configuration System
   - [ ] Kernel config key-value registry (in-RAM)

Acceptance Criteria
- Can read a static file contained in early RAMFS and print to console.

---
## Phase 6 – Sentinel AI Integration (MVP Hooks)
Goal: Instrumentation + first adaptive behavior.

Milestones
1. Telemetry Hooks
   - [ ] Syscall ring buffer (fixed size)
   - [ ] Scheduler event log
2. Simple Heuristics -> Predictive Model Interface
   - [ ] Pluggable strategy interface (function pointers)
   - [ ] Heuristic prefetch stub (logs intent only)
3. Model Loader Abstraction
   - [ ] ONNX/TFLite loader design doc
   - [ ] Memory safety sandbox boundaries draft

Acceptance Criteria
- Enabling AI hook does not alter core scheduling semantics but logs suggestion events.

---
## Phase 7 – Packaging & Distribution Foundations
Goal: Reproducible build + artifact pipeline.

Milestones
1. CI/CD
   - [ ] GitHub Actions: build ISO matrix (GCC versions)
   - [ ] Artifact upload (kernel.elf + iso)
2. Release Metadata
   - [ ] VERSION file + semantic bump script
   - [ ] Changelog automation skeleton

Acceptance Criteria
- Pushing to main on CI produces downloadable ISO artifact.

---
## Cross-Cutting Quality Tracks
Security, performance, and code health items revisited each phase.

| Track | Recurring Tasks |
|-------|-----------------|
| Security | Validate pointer bounds, ISR hardening, stack canaries (later), privilege separation design notes |
| Performance | Boot time measurement, tick latency sampling, memory footprint snapshot |
| Reliability | Panic reproduction scripts, regression harness |
| Documentation | Update ARCHITECTURE & API docs per subsystem milestone |

---
## Dependency Graph (Simplified)
```
GDT -> IDT -> PIC -> Timer -> (Scheduling)
Multiboot Info -> Phys Alloc -> Paging -> Higher-Half -> Heap
Heap -> VFS -> (Future Userland / AI Loader)
ISR + Syscalls -> Telemetry Hooks -> AI Strategies
```

---
## Suggested Working Order for Daily Sprints (Example)
1. Morning: Implement 1–2 low-risk primitives (e.g., GDT build + load)
2. Midday: Integrate + add minimal self-test (trigger an intentional fault)
3. Afternoon: Document changes + add debug visibility (print structures)
4. Evening: Refactor / cleanup + push

---
## Metrics To Start Capturing Early
- Boot time (entry -> kmain -> ready state ticks)
- Memory used by kernel sections (text/data/bss)
- Interrupt rate (timer) vs expected
- Context switch count per second (once scheduler exists)

---
## File Additions Proposed (Future PRs)
- src/kernel/gdt.asm / gdt.c (descriptor table build)
- src/kernel/idt.asm / isr.c (ISR stubs + C handlers)
- src/kernel/panic.c (central panic + register dump)
- src/kernel/printk.c (log facility)
- scripts/dev/qemu_gdb.fish (debug mode)
- scripts/dev/dump_symbols.sh (nm/objdump wrapper)

---
## Tracking Template (Copy/Paste for Issues)
```
Title: [Subsystem] Short Description
Phase: (e.g., Phase 1 – Kernel Foundations)
Goal: Single sentence value statement
Tasks:
  - [ ] Step 1
  - [ ] Step 2
Risks:
  - Detail
Definition of Done:
  - Observable criteria
Artifacts:
  - Logs / screenshots / dumps required
```

---
## Immediate Next Action Recommendation
Start Phase 0 remaining items: install toolchain packages, add CI stub, then implement GDT.

---
End of document.
