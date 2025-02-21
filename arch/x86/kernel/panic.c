#include <rotary/panic.h>

/* ------------------------------------------------------------------------- */

typedef struct {
    unsigned int eax, ebx, ecx, edx, esi, edi, esp, ebp;
} registers_t;

/* ------------------------------------------------------------------------- */

/**
 * capture_registers() - Retrieve CPU registers.
 */
void capture_registers(registers_t * regs) {
    asm volatile("mov %%eax, %0" : "=m" (regs->eax));
    asm volatile("mov %%ebx, %0" : "=m" (regs->ebx));
    asm volatile("mov %%ecx, %0" : "=m" (regs->ecx));
    asm volatile("mov %%edx, %0" : "=m" (regs->edx));
    asm volatile("mov %%esi, %0" : "=m" (regs->esi));
    asm volatile("mov %%edi, %0" : "=m" (regs->edi));
    asm volatile("mov %%esp, %0" : "=m" (regs->esp));
    asm volatile("mov %%ebp, %0" : "=m" (regs->ebp));
}

/* ------------------------------------------------------------------------- */

/**
 * kernel_panic_print_info() - Print arch-specific info during kernel panic.
 * @screen_y: The VGA line on which to start writing information.
 */
void kernel_panic_print_info(uint32_t screen_y) {
}

/* ------------------------------------------------------------------------- */
