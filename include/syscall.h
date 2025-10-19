#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <context.h>

// System call numbers
#define SYSCALL_EXIT    1
#define SYSCALL_WRITE   2
#define SYSCALL_READ    3
#define SYSCALL_YIELD   4

// Maximum number of syscalls
#define MAX_SYSCALLS    256

// System call handler
void syscall_init(void);
void syscall_handler(registers_t* regs);

// System call implementations
int sys_exit(int status);
int sys_write(int fd, const char* buf, uint32_t len);
int sys_read(int fd, char* buf, uint32_t len);
int sys_yield(void);

#endif // SYSCALL_H
