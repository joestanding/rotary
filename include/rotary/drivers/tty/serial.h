/*
 * include/rotary/drivers/tty/serial.h
 */

#ifndef INC_DRIVERS_TTY_SERIAL_H
#define INC_DRIVERS_TTY_SERIAL_H

#include <rotary/core.h>
#include <rotary/debug.h>
#include <arch/serial.h>

/* ------------------------------------------------------------------------- */

int32_t arch_serial_init();
int32_t arch_serial_write_line(uint32_t port, char * line);

int32_t serial_init();
int32_t serial_write_line(uint32_t port, char * line);
int32_t serial_set_debug_port(uint32_t port_id);
uint32_t serial_get_debug_port();

/* ------------------------------------------------------------------------- */

#endif
