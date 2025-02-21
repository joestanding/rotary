/*
 * kernel/mm/vm.c
 * Virtual Memory and Address Space Management
 *
 * Functions to manage the virtual address space of tasks. The virtual address
 * space structure contains information on all mappings within a task's
 * virtual memory, including metadata on attributes such as writability and
 * permissions. This data structure is architecture-independent, and contains
 * a pointer to an architecture-specific page table structure.
 *
 * Mappings added to a virtual address space will not necessarily immediately
 * be copied to the relevant architecture-specific page table,
 */

#include <rotary/mm/vm.h>

/* ------------------------------------------------------------------------- */
/* Address Space                                                             */
/* ------------------------------------------------------------------------- */

/**
 * vm_space_new() - Allocate and initialise a new virtual address space
 * @space: A pointer to the virtual address space structure.
 *
 * Initialises a virtual address space by initialising the list node and
 * calls upon the paging subsystem to create and allocate a new page table.
 *
 * The page table included within the address space will contain the kernel
 * mappings by default, as these are automatically added by ptable_pgd_new().
 *
 * Return: A pointer to the newly allocated address space, NULL if memory
 *         allocation fails
 */
struct vm_space * vm_space_new() {
    struct vm_space * vms = (struct vm_space*)kmalloc(sizeof(struct vm_space));
    if(!vms) {
        klog("vm_space_new(): Failed to alloc. memory!\n");
        return NULL;
    }

    llist_init(&vms->mappings);
    vms->pgd = VIR_TO_PHY(ptable_pgd_new());

    return vms;
}

/* ------------------------------------------------------------------------- */

/**
 * vm_space_destroy() - De-allocate all memory used by an address space
 * @space: A pointer to the vm_space to be destroyed.
 *
 * All memory backing the specified address space shall be de-allocated.
 * The paging subsystem will free all pages used by any sub-tables, and the
 * global page directory itself. The slab allocated memory for the address
 * space is then freed.
 */
void vm_space_destroy(struct vm_space * space) {
    /* Free the pages allocated for the page table */
    ptable_pgd_free(PHY_TO_VIR(space->pgd));
    /* Free the slab-alloced memory for the vm_space object */
    kfree(space);
}

/* ------------------------------------------------------------------------- */

/**
 * vm_space_add_map() - Add a mapping to an address space
 * @space: A pointer to the address space
 * @map:   A pointer to the mapping to add
 *
 * Adds the mapping to the address space's mapping list.
 */
void vm_space_add_map(struct vm_space * space, struct vm_map * map) {
    llist_add(&space->mappings, &map->list_node);
}

/* ------------------------------------------------------------------------- */

/**
 * vm_space_delete_map() - Remove a mapping from an address space
 * @space: A pointer to the address space
 * @map:   A pointer to the mapping to remove
 *
 * Unlinks the mapping from the address space's mapping list.
 */
void vm_space_delete_map(struct vm_space * space, struct vm_map * map) {
    llist_delete_node(&map->list_node);
}

/* ------------------------------------------------------------------------- */

/**
 * vm_space_page_fault() - Handle a page fault
 * @space:      The VM space to search for mappings and potentially update
 * @fault_addr: The address of the page fault
 *
 * Searches the VM space of the task affected by the page fault to identify
 * any mappings that contain the faulted address. If a mapping exists, invoke
 * map_page() to allocate a physical page and map it into the task's page table
 * at the relevant virtual address.
 *
 * This is used for "lazy loading" of page table entries, in which they are
 * only added to the page table when they're accessed.
 *
 * Return: E_SUCCESS if handled without error, E_ERROR otherwise
 */
int32_t vm_space_page_fault(struct vm_space * space, void * fault_addr) {
    struct vm_map *map;

    klog("vm_space_page_fault(): fault at 0x%x\n", fault_addr);

    /* Check whether there's any VM mappings in the VM space that cover the
     * fault address */
    list_for_each(map, &space->mappings, list_node) {
        klog("Map[start: 0x%x | end: 0x%x]\n", map->start_addr, map->end_addr);
        if(fault_addr >= map->start_addr && fault_addr < map->end_addr) {
            klog("Mapping contains fault address\n");
            /* The faulted address is mapped, alloc. a page & update PGD */
            return vm_space_map_page(space, fault_addr);
        }
    }

    /* TODO: handle user tasks that have page faulted on an unmapped address */

    /* No mapping was found */
    klog("No mapping found, unhandled page fault!!!\n");
    return E_ERROR;
}

/* ------------------------------------------------------------------------- */

/**
 * vm_space_map_page() - Allocate a page for a mapped address
 * @space: The VM space containing the page table to update
 * @addr:  The virtual address to add to the page table
 *
 * Called when a page fault occurs, but the current executing task has a
 * mapping for the faulted address. Allocates a physical page and adds the
 * relevant mapping for it to the VM space's page table.
 *
 * Return: E_SUCCESS if successfully mapped, E_ERROR otherwise
 */
int32_t vm_space_map_page(struct vm_space * space, void * addr) {
    struct page * new_page = page_alloc(0, PR_KERNEL);
    if(!new_page) {
        return E_ERROR;
    }

    ptable_map(PHY_TO_VIR(space->pgd), addr, PAGE_PA(new_page), 0x00);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */
/* Individual Mapping Operations                                             */
/* ------------------------------------------------------------------------- */

/**
 * vm_map_new() - Create a new, empty VM mapping
 *
 * Allocates memory for and initialises a new empty VM mapping.
 *
 * Return: A pointer to the new VM mapping
 */
struct vm_map * vm_map_new() {
    struct vm_map * map = (struct vm_map*)kmalloc(sizeof(struct vm_map));
    memset(map, 0, sizeof(struct vm_map));
    llist_init(&map->list_node);

    return map;
}

/* ------------------------------------------------------------------------- */

/**
 * vm_map_free() - Free an existing VM mapping
 */
void vm_map_destroy(struct vm_map * map) {
    kfree(map);
}

/* ------------------------------------------------------------------------- */

#include "test/vm.c"

/* ------------------------------------------------------------------------- */
