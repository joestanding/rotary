/*
 * x86 Keyboard Interface
 * Functions for arch-specific keyboard handling. Init. function is defined
 * in the architecture-agnostic header "keyboard.h"
 */

#ifndef INC_ARCH_KEYBOARD_H
#define INC_ARCH_KEYBOARD_H

#include <rotary/core/interrupts.h>
#include <arch/io_port.h>
#include <arch/vga.h>

/* ------------------------------------------------------------------------- */

void x86_driver_keyboard_isr(struct isr_registers * registers);

/* ------------------------------------------------------------------------- */

#endif
