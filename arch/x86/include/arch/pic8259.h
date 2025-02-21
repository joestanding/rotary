/*
 * arch/x86/include/arch/pic8259.c
 * Intel 8259 Programmable Interrupt Controller
 */

#ifndef INC_ARCH_PIC8259_H
#define INC_ARCH_PIC8259_H

#include <rotary/core.h>
#include <rotary/debug.h>
#include <rotary/logging.h>
#include <arch/io_port.h>

/* ------------------------------------------------------------------------- */

#define PIC_MASTER_OFFSET   32
#define PIC_SLAVE_OFFSET    40

/* PIC commands */
#define IO_CMD_PIC_EOI  0x20
#define IO_CMD_PIC_ICW1_ICW4        0x01
#define IO_CMD_PIC_ICW1_SINGLE      0x02
#define IO_CMD_PIC_ICW1_INTERVAL4   0x04
#define IO_CMD_PIC_ICW1_LEVEL       0x08
#define IO_CMD_PIC_ICW1_INIT        0x10

#define IO_CMD_PIC_ICW3_MASTER      0x04
#define IO_CMD_PIC_ICW3_SLAVE       0x02

#define IO_CMD_PIC_ICW4_8086_MODE   0x01
#define IO_CMD_PIC_ICW4_AUTO_EOI    0x02
#define IO_CMD_PIC_ICW4_BUF_SLAVE   0x08
#define IO_CMD_PIC_ICW4_BUF_MASTER  0x0C
#define IO_CMD_PIC_ICW4_SFNM        0x10

/* ------------------------------------------------------------------------- */

int32_t pic_send_eoi(uint8_t irq);
int32_t pic_init(uint8_t master_offset, uint8_t slave_offset);

/* ------------------------------------------------------------------------- */

#endif
