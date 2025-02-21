/*
 * arch/x86/kernel/io_port.c
 * x86 I/O Ports
 *
 * Read and write bytes to the x86 I/O ports using `in` and `out` instructions.
 */

#include <arch/io_port.h>

/* ------------------------------------------------------------------------- */

/**
 * io_port_in() - Read input from an x86 I/O port.
 * @port_num: The I/O port to be read from.
 *
 * Read input from an I/O port - frequently used for interactions that don't
 * use memory-mapped I/O, such as the serial controller, the 8259 Programmable
 * Interrupt Controller, and PS/2 keyboards.
 *
 * Return: Input from the specified I/O port.
 */
uint8_t io_port_in(uint16_t port_num) {
    uint8_t value;
    asm("in %%dx, %%al" : "=a"(value) : "d"(port_num));
    return value;
}

/* ------------------------------------------------------------------------- */

/**
 * io_port_out() - Write to an x86 I/O port.
 * @port_num: The I/O port to be written to.
 * @data:     The byte to be written to the I/O port.
 *
 * Writes output to an I/O port - most frequently used for configuring the 8259
 * Programmable Interrupt Controller and serial output.
 */
void io_port_out(uint16_t port_num, uint8_t data) {
    asm("out %%al, %%dx" :: "a"(data), "d" (port_num));
}

/* ------------------------------------------------------------------------- */
