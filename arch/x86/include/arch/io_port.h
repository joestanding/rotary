/*
 * arch/x86/include/arch/io_port.h
 * x86 IO Ports
 */

#ifndef INC_ARCH_IO_PORT_H
#define INC_ARCH_IO_PORT_H

#include <rotary/core.h>

/* ------------------------------------------------------------------------- */

// 8259 Programmable Interrupt Controller
#define IO_PORT_PIC1_CMD        0x0020
#define IO_PORT_PIC1_DATA       0x0021
#define IO_PORT_PIC2_CMD        0x00A0
#define IO_PORT_PIC2_DATA       0x00A1


// Programmable Interrupt Timer
#define IO_PORT_PIT_CHAN_0      0x40
#define IO_PORT_PIT_CHAN_1      0x41
#define IO_PORT_PIT_CHAN_2      0x42
#define IO_PORT_PIT_CMD         0x43

// Serial Ports
#define IO_PORT_SERIAL_COM1     0x3F8
#define IO_PORT_SERIAL_COM2     0x2F8
#define IO_PORT_SERIAL_COM3     0x3E8
#define IO_PORT_SERIAL_COM4     0x2E8
#define IO_PORT_SERIAL_COM5     0x5F8
#define IO_PORT_SERIAL_COM6     0x4F8
#define IO_PORT_SERIAL_COM7     0x5E8
#define IO_PORT_SERIAL_COM8     0x4E8

// Keyboard
#define IO_PORT_PS2_KEYBOARD    0x0060

// VGA
#define IO_PORT_VGA_HORIZ_TOTAL             0x3D4 + 0x00
#define IO_PORT_VGA_END_HORIZ_DISPLAY       0x3D4 + 0x01
#define IO_PORT_VGA_START_HORIZ_BLANKING    0x3D4 + 0x02
#define IO_PORT_VGA_END_HORIZ_BLANKING      0x3D4 + 0x03
#define IO_PORT_VGA_START_HORIZ_RETRACE     0x3D4 + 0x04
#define IO_PORT_VGA_END_HORIZ_RETRACE       0x3D4 + 0x05

/* ------------------------------------------------------------------------- */

void    io_port_out(uint16_t port_num, uint8_t data);
uint8_t io_port_in(uint16_t port_num);

/* ------------------------------------------------------------------------- */

#endif
