/*
 * arch/x86/include/arch/vga.h
 * VGA Text Mode
 */

#ifndef INC_ARCH_VGA_H
#define INC_ARCH_VGA_H

#include <arch/io_port.h>

/* ------------------------------------------------------------------------- */

#define TOP_LINE    3

#define VGA_BUFFER  0xc00b8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

/* ------------------------------------------------------------------------- */

#endif
