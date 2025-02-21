/*
 * arch/arm/kernel/sync.c
 * Synchronisation Primitives
 *
 * Provides sync. primitives such as spinlocks.
 */

#include <rotary/sync.h>

/* ------------------------------------------------------------------------- */

void lock( volatile atomic_flag * lock )
{
    while( atomic_flag_test_and_set_explicit( lock, memory_order_acquire ) )
    {
        __asm__ volatile("yield" ::: "memory");
    }
}

/* ------------------------------------------------------------------------- */

void unlock( volatile atomic_flag * lock )
{
    atomic_flag_clear_explicit( lock, memory_order_release );
}

/* ------------------------------------------------------------------------- */
