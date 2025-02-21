/*
 * arch/x86/include/arch/syscall.h
 * x86-specific Syscall Definitions
 */

#ifndef INC_ARCH_SYSCALL_H
#define INC_ARCH_SYSCALL_H

/* ------------------------------------------------------------------------- */

struct syscall_regs {
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t syscall_no;
};

/* ------------------------------------------------------------------------- */

void syscall_handler(struct syscall_regs * regs);

/* ------------------------------------------------------------------------- */

#endif
