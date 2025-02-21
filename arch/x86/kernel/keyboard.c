/*
 * arch/x86/kernel/keyboard.c
 * x86 Keyboard Handling
 *
 * Registers a handler for the x86 PS/2 keyboard interrupt and dispatches
 * input to the kernel generic input handler.
 */

#include <rotary/drivers/input/keyboard/keyboard.h>

/* ------------------------------------------------------------------------- */

/**
 * arch_keyboard_init() - Architecture-specific keyboard initialisation.
 *
 * Registers an interrupt handler for the x86 PS/2 interrupt.
 *
 * Return: E_SUCCESS
 */
int32_t arch_keyboard_init() {
    register_interrupt_handler(INT_KEYBOARD, &x86_driver_keyboard_isr);
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * x86_driver_keyboard_isr() - Handle a PS/2 keyboard interrupt.
 * @registers: A pointer to the registers at the time of interrupt.
 *
 * Reads the pressed key from the relevant PS/2 IO port and passes it to the
 * architecture-agnostic keyboard input handler.
 */
void x86_driver_keyboard_isr(struct isr_registers * registers) {
    // Retrieve pressed key from PS/2 IO port
    uint8_t keypress = io_port_in(IO_PORT_PS2_KEYBOARD);
    // Pass to kernel for handling
    keyboard_handle_input(keypress);
}

/* ------------------------------------------------------------------------- */
