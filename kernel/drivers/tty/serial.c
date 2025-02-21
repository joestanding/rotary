/*
 * kernel/serial.c
 * Serial I/O
 *
 * Initialise and handle serial input and output.
 */

#include <rotary/drivers/tty/serial.h>

/* ------------------------------------------------------------------------- */

uint8_t  serial_initialised = 0;
uint32_t serial_debug_port = 0;

/* ------------------------------------------------------------------------- */

/**
 * serial_init() - Initialise serial I/O.
 *
 * Invokes architecture-specific serial I/O initialisation, then marks
 * serial as ready-to-go.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t serial_init() {
    if(!SUCCESS(arch_serial_init())) {
        return E_ERROR;
    }

    // We're all good!
    serial_initialised = 1;
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * serial_write_line() - Print a line to the specified serial port.
 * @port: The serial port ID to write to.
 * @line: The null-terminated ASCII string to write.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t serial_write_line(uint32_t port, char * line) {
    if(!serial_initialised)
        return E_ERROR;

    if(!SUCCESS(arch_serial_write_line(port, line))) {
        return E_ERROR;
    } else {
        return E_SUCCESS;
    }
}

/* ------------------------------------------------------------------------- */

/**
 * serial_get_debug_port() - Get the ID for the debug port.
 *
 * Return: A uint32_t value identifying the debug serial port.
 */
uint32_t serial_get_debug_port() {
    return serial_debug_port;
}

/* ------------------------------------------------------------------------- */

/**
 * serial_set_debug_port() - Set the ID for the debug port.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t serial_set_debug_port(uint32_t port_id) {
    serial_debug_port = port_id;
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */
