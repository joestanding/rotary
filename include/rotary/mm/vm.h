/*
 * include/rotary/mm/vm.h
 * Virtual Memory and Address Space Management
 */

#ifndef INC_MM_VM_H
#define INC_MM_VM_H

#include <rotary/core.h>
#include <rotary/list.h>
#include <rotary/mm/kmalloc.h>
#include <arch/ptable.h>

/* ------------------------------------------------------------------------- */

#define VM_MAP_READ     0x01 /* Pages can be read from */
#define VM_MAP_WRITE    0x02 /* Pages can be written to */
#define VM_MAP_EXEC     0x04 /* Pages can be executed */
#define VM_MAP_SHARED   0x08 /* Pages are shared amongst multiple tasks */
#define VM_MAP_IO       0x10 /* Area contains device I/O space */
#define VM_MAP_RESERVED 0x20 /* Area must not be swapped out */

/* ------------------------------------------------------------------------- */

/* Operations that can be called on a VM mapping - for example, invoking
 * a page fault handler relevant to the type of mapping upon a page fault */
struct vm_ops {
    void (*placeholder) ();
};

/* Represents a task's virtual address space, containing all of its mappings,
 * and a pointer to its architecture-specific page table */
struct vm_space {
    struct pgd * pgd;      /* Pointer to the actual page table */
    list_head_t  mappings; /* A list of struct vm_map structs */
    uint32_t     users;    /* How many tasks use this address space */
};

/* Represents a region of physical memory mapped into virtual memory */
struct vm_map {
    list_node_t  list_node;
    struct vm_space * space; /* A mapping will only belong to one VM space */
    void * start_addr;       /* First (inclusive) address in the range */
    void * end_addr;         /* Final (exclusive) address in the range */
    uint32_t flags;          /* Example: read, read/write, executable */
};

/* ------------------------------------------------------------------------- */

struct vm_space * vm_space_new();
void vm_space_destroy(struct vm_space * space);

void vm_space_add_map(struct vm_space * space, struct vm_map * map);
void vm_space_delete_map(struct vm_space * space, struct vm_map * map);

int32_t vm_space_page_fault(struct vm_space * space, void * fault_addr);
int32_t vm_space_map_page(struct vm_space * space, void * addr);

/* ------------------------------------------------------------------------- */

struct vm_map * vm_map_new();
void vm_map_destroy(struct vm_map * map);
struct vm_map * vm_map_find(struct vm_space * space, void * addr);

/* ------------------------------------------------------------------------- */

#endif
