/*
 * kernel/panic.c
 * Kernel Panic
 *
 * Functions to handle unrecoverable errors within the kernel, such as
 * providing diagnostic information via serial and visual output.
 */

#include <rotary/panic.h>

/* ------------------------------------------------------------------------- */

/**
 * kernel_panic() - Trigger a kernel panic, pausing execution.
 * @message:     The primary error message to be displayed.
 * @source_file: The source file in which the panic was raised.
 * @source_func: The function in which the panic was raised.
 * @bugcheck:    Indicates whether the panic is a bugcheck that will only be
 *               raised when the kernel is compiled with debugging enabled.
 *
 * Our Blue Screen of Death equivalent - to be called when an unrecoverable
 * error occurs, such as a critical boot process failing. This will attempt
 * a debug break in emulators, and then enter an infinite loop.
 *
 * Information about the kernel panic is displayed to the user via VGA and
 * serial output for debugging.
 */
void kernel_panic(const char * message, const char * source_file,
                  const char * source_func, uint8_t bugcheck) {
    vga_clear_screen(VGA_COLOUR_BLACK);

    vga_printf_pos(0, 0, "---------------------------------------");
    vga_printf_pos(0, 1, "!!! Kernel panic !!!");
    vga_printf_pos(0, 2, "---------------------------------------");

    if(!bugcheck) {
        vga_printf_pos(0, 4, "The system encountered an unrecoverable error.");
    } else {
        vga_printf_pos(0, 4, "The system encountered a bugcheck.");
    }

    vga_printf_pos(0, 6, "Error: %s", message);

    vga_printf_pos(0, 8, "File:  %s", source_file);
    vga_printf_pos(0, 9, "Func:  %s", source_func);

    kernel_panic_print_info(10);

    debug_break();
    while(true) {}
}

/* ------------------------------------------------------------------------- */
