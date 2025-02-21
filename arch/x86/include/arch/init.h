/*
 * arch/x86/include/arch/init.h
 * x86-specific Initialisation
 *
 * Code to be run at system boot to initialise x86-specific subsystem and
 * data structures.
 */

#ifndef INC_ARCH_INIT2_H
#define INC_ARCH_INIT2_H

#include <rotary/sched/task.h>
#include <rotary/timer.h>
#include <rotary/drivers/tty/tty.h>
#include <rotary/drivers/input/keyboard/keyboard.h>
#include <rotary/mm/ptable.h>
#include <rotary/mm/bootmem.h>
#include <arch/cpuid.h>
#include <arch/multiboot.h>

/* ------------------------------------------------------------------------- */
/* Defines                                                                   */
/* ------------------------------------------------------------------------- */

#define OK_STR "[ \fOK\v ]\n"
#define FAIL_STR "[\rFAIL\v]\n"

/* ------------------------------------------------------------------------- */

#endif
