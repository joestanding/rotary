/*
 * kernel/mm/bootmem.c
 * Early Boot Memory Allocator
 *
 * Provides memory allocation capability to the kernel before the page
 * allocator is initialised. Bootmem is used by the page allocator to identify
 * memory where the page structures can be stored.
 */

#include <rotary/mm/bootmem.h>

/* External symbols from the linker provide addresses for the start and end of
 * the kernel in physical memory */
extern uintptr_t KERNEL_PHYS_START;
extern uintptr_t KERNEL_PHYS_END;

struct mem_region mem_regions[MAX_MEM_REGIONS];

uint32_t highest_pfn = 0;
uint32_t region_count = 0;

/* ------------------------------------------------------------------------- */

/**
 * bootmem_mark_free() - Mark memory as usable based on bootmem region maps.
 *
 * Initialises the bootmem subsystem to provide memory allocation capability
 * to the kernel, before the page allocation system is fully initialised.
 * The page allocator itself will make use of bootmem to find space for its
 * pre-allocated table of page structures.
 *
 * This function expects available memory regions to have been added to the
 * mem_regions array through calls to bootmem_add_mem_region(), which is to be
 * called by architecture-specific code capable of detecting usable memory
 * regions.
 *
 * This function also expects the page struct array to have been initialised
 * by the page allocator initialisation. This will set all pages to INVALID,
 * preventing their allocation. This function will clear the INVALID flag,
 * making pages available based on the memory regions provided to bootmem
 * previously.
 *
 * Additionally, bootmem_mark_free() will not make any pages containing
 * bootmem-allocated memory available. When bootmem is allocated via
 * bootmem_alloc(), the starting address of the memory region issued from is
 * adjusted to no longer include the allocated memory. This function will
 * therefore not iterate over memory allocated via bootmem_alloc(), and the
 * pages representing that memory will remain marked as INVALID.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t bootmem_mark_free() {

    /* Initialise the buddy allocator, this will request memory from bootmem
     * for page structs and initialise them - then we can mark pages as free
     * using our information on available memory regions. */
    //buddy_init(highest_pfn);

    klog("bootmem_mark_free(): Initialising bootmem..\n");
    klog("bootmem_mark_free(): Highest PFN:  %d\n", highest_pfn);
    klog("bootmem_mark_free(): Region count: %d\n", region_count);

    /* Panic if we don't seem to have any memory regions available - perhaps
     * we were called before init. code called add_mem_region()? */
    if(region_count == 0 || highest_pfn == 0) {
        PANIC("No memory regions registered in bootmem at time of "
              "mark_free()!");
        return E_ERROR;
    }

    /* Now all the page structs are initialised, we can mark available pages
     * as free using our memory maps. As the start_addr attribute for each
     * region is incremented when boot memory is allocated from it, this
     * ensures that these allocated regions will not be marked as available
     * in the following code. */
    uint32_t freed_pages = 0;
    for(uint32_t i = 0; i < region_count; i++) {
        if(mem_regions[i].start_addr == 0)
            continue;

        klog("bootmem_mark_free(): Processing mem. region %d "
                          "(start: 0x%x | end: 0x%x)\n",
                          i, mem_regions[i].start_addr,
                          mem_regions[i].end_addr);

        /* Ensure we're working with page aligned addresses */
        uintptr_t first_page = PAGE_ALIGN(mem_regions[i].start_addr);
        uintptr_t end_page   = PAGE_ALIGN_DOWN(mem_regions[i].end_addr);

        /* For each page in this memory section, mark it as free */
        for(; first_page < end_page; first_page += PAGE_SIZE) {

            /* Retrieve a handle to the buddy page */
            uint32_t pfn = PA_TO_PFN(first_page);
            struct page * page = page_from_pfn(pfn);

            /* Remove the INVALID flag */
            CLEAR_BIT(page->flags, PF_INVALID);

            page_initial_free(page);

            freed_pages++;
        }
    }

    klog("bootmem_mark_free(): Freed %d pages of %d total\n",
                      freed_pages, highest_pfn);

    klog("\n");
    // paging_print_debug();
    klog("\n");
    buddy_print_debug();
    klog("\n");
    bootmem_print_debug();
    klog("\n");

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * bootmem_add_mem_region() - Register a region of memory.
 * @start_addr: The starting address of the memory region.
 * @end_addr: The ending address of the memory region.
 * @type: The type of the memory region (MEM_REGION_RESERVED or
 *        MEM_REGION_AVAILABLE).
 *
 * Registers a region of memory, available or invalid. This function is to be
 * called by architecture-specific code, such as the Multiboot processing code
 * on x86, which makes use of memory maps passed through by GRUB.
 *
 * This function will also ensure that memory regions containing kernel code
 * are adjusted to avoid the kernel code region, but preserve the other areas.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t bootmem_add_mem_region(uintptr_t start_addr, uintptr_t end_addr,
                               uint32_t type) {
    /* Ensure we're working with page aligned addresses - align down for the
     * end address so we don't end up including a bit more space than we were
     * allowed to as a side effect */
    start_addr = PAGE_ALIGN(start_addr);
    end_addr   = PAGE_ALIGN_DOWN(end_addr);
    uintptr_t kernel_start = PAGE_ALIGN(&KERNEL_PHYS_START);
    uintptr_t kernel_end   = PAGE_ALIGN(&KERNEL_PHYS_END);

    klog("add_region(): Request w/ start_addr: 0x%x, end_addr: 0x%x\n",
         start_addr, end_addr);

    /* Check whether we have space in the memory region array */
    if(region_count == MAX_MEM_REGIONS) {
        klog("add_region(): No space left in region array!\n");
        return E_ERROR;
    }

    klog("start_addr: 0x%x kernel_start: 0x%x end_addr: 0x%x kernel_end: 0x%x\n",
            start_addr, kernel_start, end_addr, kernel_end);

    /* Check if the region is entirely within kernel memory */
    if(start_addr >= kernel_start && end_addr < kernel_end) {
        klog("add_region(): Region entirely in kernel memory!\n");
        return E_ERROR;
    }

    /* Ensure a valid type has been provided */
    if(type != MEM_REGION_RESERVED && type != MEM_REGION_AVAILABLE) {
        klog("add_region(): Invalid type (%d) provided!\n", type);
        return E_ERROR;
    }

    /* Validate start and end addresses are in order */
    if(start_addr > end_addr) {
        klog("add_region(): Start/end addresses are not in order\n");
        return E_ERROR;
    }

    /* Ensure the region is of valid size */
    if((start_addr == end_addr) ||
       (end_addr - start_addr < PAGE_SIZE)) {
        klog("add_region(): Region is of invalid size!\n");
        return E_ERROR;
    }

    /* Ensure that the provided memory region does not overlap with the kernel
     * image in memory - if it does, we can adjust its bounds so it doesn't */
   if (start_addr < kernel_start && end_addr > kernel_end) {
        klog("add_region(): Region surrounds kernel memory, splitting..\n");
        bootmem_add_mem_region(start_addr, kernel_start, type);
        bootmem_add_mem_region(kernel_end, end_addr, type);
        return E_SUCCESS;
    } else if (start_addr < kernel_start && end_addr > kernel_start) {
        klog("add_region(): Overlaps at beginning of kernel memory\n");
        end_addr = kernel_start;
    } else if (start_addr < kernel_end && end_addr > kernel_end) {
        klog("add_region(): Overlaps at end of kernel memory\n");
        start_addr = kernel_end;
    }

    /* Keep a record of the highest page number, so we know how much memory
     * is usable in the system and how many page structures we'll need to
     * create */
    uint32_t pfn = PA_TO_PFN(end_addr);
    if(pfn > highest_pfn) {
        highest_pfn = pfn;
    }

    klog("add_region(): Adding region: 0x%x -> 0x%x\n",
                      start_addr, end_addr);

    /* Find a free memory region in the array and update it */
    for(uint32_t i = 0; i < MAX_MEM_REGIONS; i++) {
        if(mem_regions[i].start_addr == 0x00) {
            memset(&mem_regions[i], 0, sizeof(struct mem_region));
            mem_regions[i].start_addr = start_addr;
            mem_regions[i].end_addr   = end_addr;
            mem_regions[i].type       = type;
            mem_regions[i].orig_start_addr = start_addr;
            klog("add_region(): New region ID: %d\n", i);
            region_count++;
            return E_SUCCESS;
        }
    }

    /* If we've run out of memory regions in the array, return an error */
    klog("add_region(): Ran out of memory regions!\n");
    return E_ERROR;
}

/* ------------------------------------------------------------------------- */

/**
 * bootmem_alloc() - Allocate memory from the bootmem pool with alignment.
 * @size:      The amount of memory to be allocated.
 * @alignment: The alignment requirement for the allocated memory.
 *
 * Allocates memory from the bootmem pool, considering the specified alignment.
 * Iterates through each registered memory region to find a region that has
 * sufficient available memory and satisfies the alignment constraint.
 *
 * The function ensures that the returned memory address is aligned to the
 * specified alignment value by adjusting the starting address of the memory
 * region if necessary.
 *
 * If a suitable region is found, the region's starting address is updated to
 * reflect the allocated space, and a virtual address pointer to the allocated
 * memory is returned. If no region can satisfy the request, the function
 * returns `NULL`.
 *
 * Return: A pointer to the allocated memory if successful, or `NULL` if no
 *         memory was available to sufficiently fulfill the request.
 */
void * bootmem_alloc(size_t size, size_t alignment) {
    klog("bootmem_alloc(): %d bytes requested with %d alignment\n", size, alignment);

    for (uint32_t i = 0; i < MAX_MEM_REGIONS; i++) {
        if (mem_regions[i].type != MEM_REGION_AVAILABLE)
            continue;

        uintptr_t aligned_start = ALIGN(mem_regions[i].start_addr, alignment);
        if (aligned_start >= mem_regions[i].end_addr)
            continue; // Skip if alignment exceeds region bounds

        uintptr_t region_size = mem_regions[i].end_addr - aligned_start;

        if (region_size >= size) {
            void *alloc_start = (void *)aligned_start;
            mem_regions[i].start_addr = aligned_start + size;

            klog("bootmem_alloc(): Returning allocation at %x\n",
                  PHY_TO_VIR(alloc_start));
            return PHY_TO_VIR(alloc_start);
        }
    }

    klog("bootmem_alloc(): No regions could satisfy request!\n");
    return NULL;
}



/* ------------------------------------------------------------------------- */

/**
 * bootmem_reset() - Reset the bootmem allocator state.
 *
 * Primarily intended for test code that may require resetting of the bootmem
 * allocator, while not actually being a bootmem test, and therefore does not
 * have direct access to bootmem variables and data structures. For example,
 * the buddy allocator test suite will want to ensure that bootmem is reset 
 * for each unit test, as buddy_init() requests memory from bootmem to store 
 * its page structures.
 */
void bootmem_reset() {
    memset(mem_regions, 0, sizeof(mem_regions));
    highest_pfn  = 0;
    region_count = 0;
}

/* ------------------------------------------------------------------------- */

/**
 * bootmem_highest_pfn() - Return the highest page frame number observed.
 *
 * Return: The highest page frame number available.
 */
uint32_t bootmem_highest_pfn() {
    return highest_pfn;
}

/* ------------------------------------------------------------------------- */

/**
 * bootmem_print_debug() - Print debug information about bootmem.
 */
void bootmem_print_debug() {
    klog("--- Bootmem Info ---\n");
    klog("Memory Regions:      %d\n", region_count);
    klog("Highests Phys. Addr: 0x%x\n", PFN_TO_PA(highest_pfn));
    for(uint32_t i = 0; i < MAX_MEM_REGIONS; i++) {
        if(mem_regions[i].start_addr != 0x00) {
            uintptr_t bytes_used = mem_regions[i].start_addr -
                                   mem_regions[i].orig_start_addr;
            klog("Region[%d]: 0x%x -> 0x%x (%d bytes alloced.)\n",
                              i, mem_regions[i].start_addr,
                              mem_regions[i].end_addr,
                              bytes_used);
        }
    }
}

/* ------------------------------------------------------------------------- */

/* Include unit tests */
#include "test/bootmem.c"

/* ------------------------------------------------------------------------- */
