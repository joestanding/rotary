/*
 * arch/x86/kernel/sync.c
 * Synchronisation Primitives
 *
 * Provides sync. primitives such as spinlocks.
 */

#include <rotary/sync.h>

/* ------------------------------------------------------------------------- */

/**
 * lock() - Lock a spinlock.
 * @lock: A handle to the spinlock to be locked.
 */
void lock( volatile atomic_flag * lock )
{
    while( atomic_flag_test_and_set_explicit( lock, memory_order_acquire ) )
    {
        asm( "pause" );
    }
}

/* ------------------------------------------------------------------------- */

/**
 * unlock() - Unlock a spinlock.
 * @lock: A handle to the spinlock to be unlocked.
 */
void unlock( volatile atomic_flag * lock )
{
    atomic_flag_clear_explicit( lock, memory_order_release );
}

/* ------------------------------------------------------------------------- */
