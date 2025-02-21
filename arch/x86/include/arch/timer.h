/*
 * arch/x86/include/arch/timer.h
 * Programmable Interval Timer
 */

#ifndef INC_ARCH_TIMER_H
#define INC_ARCH_TIMER_H

#include <rotary/core.h>
#include <rotary/sched/task.h>
#include <rotary/logging.h>
#include <rotary/debug.h>
#include <rotary/core/interrupts.h>
#include <arch/io_port.h>

/* ------------------------------------------------------------------------- */

int32_t timer_init();
int32_t timer_tick();

/* ------------------------------------------------------------------------- */

#endif
