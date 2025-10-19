# 🌌 Aether OS

> **The fully customizable, AI-native operating system built from scratch — designed for creative professionals, developers, and dreamers of intelligent systems.**

Aether OS is a next-generation operating system built entirely from **scratch** in **Assembly, C, and Python**, with **AI and ML infused at every layer**. Inspired by the customizability of Arch Linux and the intelligence of modern AI systems, Aether is a modular, secure, and adaptive OS that evolves with its users.

---

## ⚙️ Core Philosophy

- **From Scratch**: No Linux, no Unix base. Aether is built from the bootloader up.
- **Fully Customizable**: Like Arch, but more powerful — build only what you need.
- **AI-Native by Design**: AI is not a plugin. It's at the heart of the OS (Sentinel AI).
- **Designed for Real Users**: Creative professionals and developers — not just hobbyists.
- **Local + Cloud AI**: Hybrid AI model with privacy-first local intelligence and cloud extension.

---

## 🧠 Sentinel AI — The Brain of Aether

Sentinel AI is Aether’s integrated intelligence layer:

- Learns user behavior & adapts UI, performance, and workflows
- Powers natural language system commands & contextual understanding
- Uses ML for memory optimization, prefetching, auto-tagging, and anomaly detection
- Local models for privacy, cloud extensions for power
- Offers explainable actions — logs everything it changes or automates

---

## 🧩 Key Features

| Feature | Description |
|--------|-------------|
| 🏗️ Built From Scratch | Bootloader → Kernel → Userland written in Assembly, C, and Python |
| 🎛️ Modular & Customizable | CLI/GUI, system services, AI layers — install only what you want |
| 🧠 AI/ML Everywhere | Integrated into the kernel, UI, CLI, and services |
| 🔄 Rolling Release | Bleeding-edge updates, atomic install, and rollback support |
| 🧬 Adaptive System | Context-aware system optimization and workflow learning |
| 🗣️ Natural Interfaces | Voice & chat-based command interface |
| 🔐 Secure by Design | Sandboxed AI modules, signed packages, differential privacy AI training |
| 📦 Smart Package Manager | Lightweight, AI-powered tool inspired by pacman |
| 📱 Future-Proof | Will support embedded platforms and tiny ML in later phases |

---

## 🛠️ Tech Stack

| Layer | Tech |
|------|------|
| Bootloader | Assembly (16/32/64-bit), GRUB or custom |
| Kernel | C + Assembly |
| Drivers & Memory | C |
| Shell & CLI | Custom or bash/zsh/fish compatible |
| GUI | Qt, GTK, or web-based |
| AI/ML | ONNX Runtime, TensorFlow Lite, PyTorch (training), TinyML (embedded) |
| Language Support | C, Assembly, Python, some C++ for GUI/ML |
| Package System | C/Python hybrid, supports signed packages & auto-dependencies |

---

## 🚀 Project Roadmap

### 🧱 Phase 1: Core Bootloader & Kernel ✓
- [x] Custom bootloader (x86 GRUB Multiboot)
- [x] Kernel entry point in C
- [x] VGA text mode console
- [x] Interrupt handling (IDT)
- [x] Timer (PIT - 100 Hz)
- [x] Keyboard driver (PS/2)
- [x] Basic shell

### 🖥️ Phase 2: Memory Management ✓
- [x] Kernel heap allocator (4MB)
- [x] kmalloc/kfree implementation
- [x] Shell enhancements (colored output, commands)
- [x] Panic handler with register dump

### 🧠 Phase 3: System Information ✓
- [x] CPU detection
- [x] Memory info display
- [x] System stats (uptime, ticks)
- [x] Color-coded logging

### 🌐 Phase 4: Virtual Memory & Process Management ✓
- [x] Paging system (4KB pages, identity mapping)
- [x] Process Control Blocks (256 processes, 4KB stacks)
- [x] Context switching (assembly)
- [x] Task State Segment (TSS) for safe context switching
- [x] Round-robin scheduler (10 tick quantum)
- [x] Preemptive multitasking
- [x] Shell commands: paging, ps, sched
- [x] **Successfully runs 5+ processes simultaneously!**

### 📦 Phase 5: User Mode & System Calls (NEXT)
- [ ] Ring 3 execution for processes
- [ ] System call interface (INT 0x80)
- [ ] fork(), exec(), wait(), exit()
- [ ] Separate address spaces per process
- [ ] Inter-process communication

### � Phase 6: Sentinel AI (MVP)
- [ ] Local ML model runtime (ONNX/TFLite)
- [ ] Natural language CLI/GUI agent
- [ ] Smart system suggestions
- [ ] Package manager with AI

### 🌐 Phase 7: Cloud AI & Intelligence
- [ ] Hybrid AI (local/cloud) with fallback
- [ ] Cloud orchestration (for large model tasks)
- [ ] Federated learning implementation

### 📦 Phase 8: Embedded Target Support
- [ ] ARM/RISC-V port
- [ ] TinyML integration (TFLite Micro)
- [ ] Resource-aware UI + AI services

---

## 📚 Learn More

- [📖 Full Documentation](./docs/)
- [🧠 Research Report](./RESEARCH.md)
- [🧪 Testing Guide](./TESTING.md) - Comprehensive test procedures for Phase 4
- [✅ Phase 4 Complete](./PHASE4_COMPLETE.md) - Technical details of multitasking implementation
- [📜 License](./LICENSE)
- [📢 Follow Development](https://github.com/yourusername/aether-os)

---

## 🎮 Quick Start

### Building AetherOS

```bash
# Clone the repository
git clone https://github.com/yourusername/aether-os.git
cd aether-os

# Build the kernel
make

# Run in QEMU
make run
# or manually:
qemu-system-i386 -cdrom build/aether.iso -m 32M
```

### Available Shell Commands

**Paging (Virtual Memory):**
```
paging status    - Show page directory info
paging test      - Display page table entries
paging enable    - Activate virtual memory
```

**Process Management:**
```
ps              - List all processes
ps list         - List all processes
ps current      - Show current process
ps info <pid>   - Show process details
ps create       - Create test processes
```

**Scheduler:**
```
sched           - Show scheduler stats
sched stats     - Show scheduler stats
sched start     - Enable multitasking
sched stop      - Disable multitasking
sched yield     - Yield to next process
```

**System:**
```
help            - Show available commands
clear           - Clear screen
cpu             - Show CPU information
mem             - Show memory usage
uptime          - Show system uptime
echo <text>     - Print text
```

---

## 🤝 Contributing

We’re building an operating system from the ground up, and your help is welcome!

> 💡 Developers, designers, AI researchers — join the movement.

- Start by reading the [Contribution Guide](./CONTRIBUTING.md)
- Open a discussion or issue
- Submit pull requests for anything — from docs to kernel code

---

## 🧙 About the Project

Aether OS is more than just code — it's a **philosophy**.
> “Your system should evolve with you, not against you.”

This is a community-driven dream of crafting an AI-first operating system for creators and builders. It’s built by those who believe that the future OS isn't just a tool — it’s a co-pilot.

---

## 🛡 License

Aether OS is licensed under the [MIT License](./LICENSE). You're free to use, modify, and share — just give credit.

---

## 🔗 Contact & Community

- Twitter/X: [@aetheros_ai](https://x.com/ather_os_ai)

---

