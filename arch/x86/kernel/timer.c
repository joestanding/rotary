/*
 * arch/x86/kernel/timer.c
 * x86 Programmable Interval Timer
 *
 * Initialise the x86 PIT and handle the PIT interrupt.
 */

#include <arch/timer.h>

uint64_t ticks = 0;

/* ------------------------------------------------------------------------- */

/**
 * timer_init() - Initialise the Programmable Interval Timer.
 *
 * Initialises the Programmable Interval Timer by assigning its divisor, which
 * configures how frequently it fires.
 *
 * Return: E_SUCCESS
 */
int32_t timer_init() {
    klog("Configuring Programmable Interrupt Timer interval "
           "to 1Hz\n");
    uint32_t divisor = 1193180 / 1;

    io_port_out(IO_PORT_PIT_CMD, 0x36);

    uint8_t l = (uint8_t) (divisor & 0xFF);
    uint8_t h = (uint8_t) (divisor >> 8 & 0xFF);

    io_port_out(IO_PORT_PIT_CHAN_0, l);
    io_port_out(IO_PORT_PIT_CHAN_0, h);

    klog("Registering handler for Programmable Interrupt Timer "
           "at 0x%x\n", timer_tick);
    register_interrupt_handler(INT_PIT, &timer_tick);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * timer_tick() - Handles a Programmable Interval Timer tick.
 *
 * Increments the tick counter and triggers the task scheduler.
 *
 * Return: E_SUCCESS
 */
int32_t timer_tick() {
    ticks++;
    task_schedule();

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */
