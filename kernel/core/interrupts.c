/*
 * kernel/core/interrupts.c
 * Architecture-independent Interrupt Handlers
 *
 * Handle interrupts in an architecture-independent manner, for example
 * handling page faults on any platform.
 */

#include <rotary/core/interrupts.h>

/* ------------------------------------------------------------------------- */

void int_handle_page_fault(void * fault_addr) {
    debug_break(); 
}

/* ------------------------------------------------------------------------- */
