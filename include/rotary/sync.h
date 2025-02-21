/*
 * include/rotary/sync.h
 * Synchronisation Primitives
 */

#ifndef INC_SYNC_H
#define INC_SYNC_H

#include <stdatomic.h>
#include <rotary/core.h>

/* ------------------------------------------------------------------------- */

void lock(volatile atomic_flag * lock);
void unlock(volatile atomic_flag * lock);

/* ------------------------------------------------------------------------- */

#endif
