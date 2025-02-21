#pragma once

static inline int write(int fd, const void *buf, unsigned int count) {
    int ret;
    asm volatile (
        "movl $0x01, %%eax\n"  // Replace N with your syscall number for write
        "movl %1, %%edi\n"
        "movl %2, %%esi\n"
        "movl %3, %%edx\n"
        "int $0x40\n"       // Syscall interrupt vector 0x40
        "movl %%eax, %0\n"
        : "=r" (ret)
        : "r" (fd), "r" (buf), "r" (count)
        : "eax", "edi", "esi", "edx"
    );
    return ret;
}
