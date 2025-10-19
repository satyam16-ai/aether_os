#ifndef USERLIB_H
#define USERLIB_H

#include <stdint.h>

// User-space syscall wrappers
// These use inline assembly to invoke INT 0x80

// Exit the process
static inline void exit(int status) {
    asm volatile(
        "mov $1, %%eax\n"      // Syscall number 1 (exit)
        "mov %0, %%ebx\n"      // Argument: status
        "int $0x80"            // Invoke syscall
        :
        : "r"(status)
        : "eax", "ebx"
    );
    // Never returns
    while(1);
}

// Write to file descriptor
static inline int write(int fd, const char* buf, uint32_t len) {
    int ret;
    asm volatile(
        "mov $2, %%eax\n"      // Syscall number 2 (write)
        "mov %1, %%ebx\n"      // Argument 1: fd
        "mov %2, %%ecx\n"      // Argument 2: buf
        "mov %3, %%edx\n"      // Argument 3: len
        "int $0x80\n"          // Invoke syscall
        "mov %%eax, %0"        // Get return value
        : "=r"(ret)
        : "r"(fd), "r"(buf), "r"(len)
        : "eax", "ebx", "ecx", "edx"
    );
    return ret;
}

// Read from file descriptor
static inline int read(int fd, char* buf, uint32_t len) {
    int ret;
    asm volatile(
        "mov $3, %%eax\n"      // Syscall number 3 (read)
        "mov %1, %%ebx\n"      // Argument 1: fd
        "mov %2, %%ecx\n"      // Argument 2: buf
        "mov %3, %%edx\n"      // Argument 3: len
        "int $0x80\n"          // Invoke syscall
        "mov %%eax, %0"        // Get return value
        : "=r"(ret)
        : "r"(fd), "r"(buf), "r"(len)
        : "eax", "ebx", "ecx", "edx"
    );
    return ret;
}

// Yield CPU to another process
static inline void yield(void) {
    asm volatile(
        "mov $4, %%eax\n"      // Syscall number 4 (yield)
        "int $0x80"            // Invoke syscall
        :
        :
        : "eax"
    );
}

// Helper: strlen
static inline uint32_t strlen(const char* str) {
    uint32_t len = 0;
    while (str[len]) len++;
    return len;
}

// Helper: print string to stdout
static inline void print(const char* str) {
    write(1, str, strlen(str));
}

#endif // USERLIB_H
