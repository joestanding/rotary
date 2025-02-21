/*
 * arch/x86/include/arch/task.h
 * Architecture-specific Task Set-up
 */

#ifndef INC_ARCH_TASK_H
#define INC_ARCH_TASK_H

#include <rotary/core/interrupts.h>
#include <arch/tss.h>
#include <arch/gdt.h>

/* ------------------------------------------------------------------------- */

struct arch_data {
};

/* Used to access stored registers and return values on the stack
 * during a task context switch. */
struct task_context {
    uint32_t edi; // Stored by task_switch
    uint32_t esi; // Stored by task_switch
    uint32_t ebx; // Stored by task_switch
    uint32_t ebp; // Stored by task_switch
    void *   eip; // Return value stored during call to task_switch
};

/* ------------------------------------------------------------------------- */

#endif
