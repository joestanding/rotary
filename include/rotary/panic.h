/*
 * include/rotary/panic.h
 * Kernel Panic for Irrecoverable Errors
 */

#ifndef INC_PANIC_H
#define INC_PANIC_H

#include <rotary/core.h>
#include <rotary/vga.h>

/* ------------------------------------------------------------------------- */

#define PANIC(msg) kernel_panic(msg, __FILE__, __func__, 0)
#ifdef KCONF_DEBUG
    #define BUG(msg) kernel_panic(msg, __FILE__, __func__, 1)
#else
    #define BUG(msg) do {} while(0)
#endif

/* ------------------------------------------------------------------------- */

void kernel_panic(const char * message, const char * source_file,
                  const char * source_func, uint8_t bugcheck);
void kernel_panic_print_info(uint32_t screen_y);
void dump_subsystems();

/* ------------------------------------------------------------------------- */

#endif
