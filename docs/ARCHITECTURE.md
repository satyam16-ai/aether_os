# Aether OS – Architecture Overview (Early Draft)

## Layered Overview

1. Boot Chain
   - Stage 1: Custom boot sector (current pretty logo)
   - Stage 2 (planned): Real-mode -> Protected-mode transition, memory map, kernel load
   - Alternative Path (implemented now): GRUB + Multiboot header (faster iteration)
2. Kernel Core
   - Entry (ASM) -> kmain (C)
   - Planned Initialization Order: GDT -> IDT -> Interrupt Controller -> Paging -> Heap -> Scheduler -> Syscalls -> VFS -> Drivers
3. Subsystems (Planned)
   - Memory: Physical + Virtual + Allocators
   - Tasking: Processes / Threads, Context Switch
   - IPC: Message Queues / Channels
   - Filesystem: VFS layer + initial RAMFS
   - Drivers: Console (VGA), Timer (PIT/HPET), Keyboard, Storage (ATA/AHCI), NIC
4. Userland (Future)
   - Shell (AI-augmented), Package Manager, Runtime Services
5. Sentinel AI Layer (Research & Iterative Integration)
   - Hooks in Scheduler (usage patterns)
   - Memory Optimizer (prefetch + anomaly detection) hooks
   - Telemetry (privacy-preserving local analytics) -> Model inference

## AI Integration Strategy (Incremental)

| Phase | Kernel Hook | AI Feature | Notes |
|-------|-------------|------------|-------|
| 1 | Boot banner | Branding / version info | Non-functional aesthetic |
| 2 | Syscall trace buffer | Basic pattern logging | Lightweight ring buffer |
| 3 | Scheduler decisions | Predictive wake/preemption | Start with heuristic model |
| 4 | Page cache manager | Prefetch scoring | ONNX/TFLite micro-model |
| 5 | Resource policy | Adaptive throttling | Feedback loop w/ user prefs |

## Memory Layout (Current Minimal)
```
0x00007C00  Boot sector loaded by BIOS
0x00100000  GRUB loads kernel (ELF) beginning at 1 MiB (via linker script)
```

Future custom loader will place kernel at 1MiB manually after enabling A20 & switching modes.

## Near-Term Tasks
- [ ] Add GDT + Protected Mode confirmation banner
- [ ] Implement IDT + basic ISR stubs (CPU exceptions)
- [ ] PIT timer + tick counter
- [ ] Keyboard driver (scancode -> ASCII)
- [ ] Framebuffer abstraction
- [ ] Physical memory bitmap (from Multiboot memory map)
- [ ] Paging enable (identity + higher half plan draft)
- [ ] Simple heap allocator (bump -> free list)

## Coding Standards (Initial)
- Freestanding C (no libc). Provide minimal replacements where needed.
- All kernel symbols: snake_case for functions, g_ prefix for globals.
- Keep early boot messages short & clear.
