/*
 * include/rotary/mm/kmalloc.h
 */

#ifndef INC_MM_KMALLOC_H
#define INC_MM_KMALLOC_H

#include <rotary/core.h>
#include <rotary/logging.h>
#include <rotary/list.h>
#include <rotary/mm/slab.h>

/* ------------------------------------------------------------------------- */

void *  kmalloc(uint32_t size);
int32_t kfree(void * addr);
void    kmalloc_print_debug();

/* ------------------------------------------------------------------------- */

#endif
