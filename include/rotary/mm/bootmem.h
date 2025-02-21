/*
 * include/rotary/mm/bootmem.h
 * Early Boot Memory Allocator
 *
 * Provides memory allocation capability to the kernel before the page
 * allocator is initialised. Bootmem is used by the page allocator to identify
 * memory where the page structures can be stored.
 */

#ifndef INC_MM_BOOTMEM_H
#define INC_MM_BOOTMEM_H

#include <rotary/core.h>
#include <rotary/logging.h>
#include <rotary/panic.h>
#include <rotary/mm/palloc.h>
#include <arch/paging.h>

/* ------------------------------------------------------------------------- */

#define MAX_MEM_REGIONS 16

#define MEM_REGION_RESERVED  0
#define MEM_REGION_AVAILABLE 1

#define BM_NO_ALIGN 1

/* ------------------------------------------------------------------------- */

struct mem_region {
    uintptr_t start_addr;
    uintptr_t end_addr;
    uintptr_t orig_start_addr;
    uint32_t  type;
};

/* ------------------------------------------------------------------------- */

int32_t  bootmem_mark_free();
int32_t  bootmem_add_mem_region(uintptr_t start_addr, uintptr_t end_addr,
         uint32_t type);
void *   bootmem_alloc(size_t size, size_t alignment);
void     bootmem_reset();
uint32_t bootmem_highest_pfn();
void     bootmem_print_debug();

/* ------------------------------------------------------------------------- */

#endif
