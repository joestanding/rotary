/*
 * include/rotary/debug.h
 */

#ifndef INC_DEBUG_H
#define INC_DEBUG_H

/* ------------------------------------------------------------------------- */

#define debug_break() asm("xchg %bx, %bx");

/* ------------------------------------------------------------------------- */

#endif
