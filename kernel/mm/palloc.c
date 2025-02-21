/*
 * kernel/mm/palloc.c
 * Buddy Page Allocator
 */

#include <rotary/mm/palloc.h>

/* Symbols pointing to the beginning and end of the kernel's image in
 * physical memory, as provided by the linker script */
extern uintptr_t KERNEL_PHYS_START;
extern uintptr_t KERNEL_PHYS_END;

uint32_t page_count;
uint32_t low_pages  = 0;
uint32_t high_pages = 0;

/* Buddy allocator */
struct block_list blocks[ORDER_MAX + 1];
struct buddy_allocator buddy_allocator;

/* ------------------------------------------------------------------------- */

/**
 * page_alloc() - Allocate a block of pages of a given order.
 * @order: The order of the block to allocate, block size is 2^order pages.
 * @flags: Reserved for future use.
 *
 * Attempts to allocate a block of pages from the block list of the desired
 * order. If no blocks are available, it attempts to split a larger block by
 * calling buddy_split_block(). The buddy_split_block() function will
 * recursively search for larger blocks to split until the maximum order is
 * reached.
 *
 * Return: A pointer to a page object representing the allocated page(s),
 *         or NULL if the allocation was unsuccessful.
 */
struct page * page_alloc(uint32_t order, uint32_t flags) {
    klog("page_alloc(): Request with order %d flags %x\n",
                      order, flags);

    /* Validate provided order */
    if(order < ORDER_MIN || order > ORDER_MAX) {
        klog("page_alloc(): Invalid order requested: %d!\n", order);
        return NULL;
    }

    lock(&buddy_allocator.lock);

    /* Check whether there's any blocks of the desired order */
    if(buddy_allocator.blocks[order].free_count == 0) {
        /* Attempt to split a larger block */
        buddy_split_block(order + 1);

        /* If no blocks are free after splitting, abort */
        if(buddy_allocator.blocks[order].free_count == 0) {
            klog("No solution found, aborting!\n");
            unlock(&buddy_allocator.lock);
            return NULL;
        }
    }

    /* Retrieve the last block page entry in the order list and return it */
    struct page * last_page = page_get_last(&buddy_allocator, order);
    if(last_page != NULL) {
        buddy_remove_block(last_page);
        klog("page_alloc(): returning page %d (paddr 0x%x, vaddr 0x%x), "
             "order %d\n", last_page->pfn, PFN_TO_PA(last_page->pfn),
             PHY_TO_VIR(PFN_TO_PA(last_page->pfn)), order);
        PAGE_INC_USES(last_page);
        unlock(&buddy_allocator.lock);
        return last_page;
    } else {
        klog("page_alloc(): failed to get last page from list w/ order %d!",
             order);
        unlock(&buddy_allocator.lock);
        return NULL;
    }
}

/* ------------------------------------------------------------------------- */

/**
 * page_free() - Free a block of pages.
 * @current_page: The first page to free.
 * @order:        The order of the block being freed, where block size is
 *                2^order pages.
 *
 * Frees a page by adding it back to its free list. Calls buddy_merge_block()
 * to recursively merge the block with its buddy to create larger free blocks,
 * minimising fragmentation.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t page_free(struct page * current_page, int order) {
    if(!current_page) {
        klog("page_free(): Received NULL page pointer!\n");
        return E_ERROR;
    }

    klog("page_free(): freeing page %d, pa 0x%x, order %d\n",
         current_page->pfn, PAGE_PA(current_page), order);

    /* Ensure we don't free a page belonging to the kernel */
    if(page_is_critical(current_page)) {
        klog("page_free(): Attempted to free kernel page!\n");
        return E_ERROR;
    }

    /* If the block is in use by multiple users, don't free */
    if(current_page->use_count > 1) {
        klog("page we're freeing has use count of above 1, decrementing..\n");
        lock(&buddy_allocator.lock);
        PAGE_DEC_USES(current_page);
        unlock(&buddy_allocator.lock);
        return E_SUCCESS;
    }

    /* Attempt to merge the current block with its buddy, until the largest
     * possible block size is reached */
    lock(&buddy_allocator.lock);
    current_page->order = order;
    buddy_merge_block(current_page, order);
    PAGE_DEC_USES(current_page);
    unlock(&buddy_allocator.lock);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * page_initial_free() - Adds a page to the buddy allocator
 * @page: The page to add to the buddy allocator.
 *
 * During initialisation of the buddy allocator, page_free() is called on all
 * available pages, which in turn attempts to merge the page into larger blocks
 * and adds them to the buddy allocator. At this time, the use count for the
 * pages is 0, but the regular page_free() function raises a bugcheck for
 * this scenario. This function does not perform a check on the use count,
 * nor does it change it.
 */
void page_initial_free(struct page * page) {
    lock(&buddy_allocator.lock);
    buddy_merge_block(page, 0);
    unlock(&buddy_allocator.lock);
}

/* ------------------------------------------------------------------------- */

/**
 * page_from_pfn() - Retrieve a page from its page frame number.
 * @pfn: The page frame number.
 *
 * Return: A pointer to the page if success, E_ERROR if PFN is invalid.
 */
struct page * page_from_pfn(uint32_t pfn) {
    struct page * page = (struct page*)buddy_allocator.page_area;
    page += pfn;
    return page;
}

/* ------------------------------------------------------------------------- */

/**
 * page_is_critical() - Returns whether a page contains critical kernel data.
 * @page: Pointer to the page to be checked.
 *
 * Determines whether the given page contains critical kernel data, such as
 * kernel code or any other data that must never be freed.
 *
 * Return: 1 if the page contains the kernel, 0 if it does not.
 */
int32_t page_is_critical(struct page * page) {
    if (!page) {
        klog("page_is_critical(): Received NULL page pointer!\n");
        return E_ERROR;
    }

    uintptr_t page_phys_addr = PFN_TO_PA(page->pfn);

    /* Check if the page belongs to the kernel or the page structure area */
    if (page_phys_addr >= (uintptr_t)&KERNEL_PHYS_START &&
        page_phys_addr < (uintptr_t)&KERNEL_PHYS_END) {
        klog("page_is_critical(): Page 0x%x belongs to kernel memory!\n",
             page_phys_addr);
        return E_ERROR;
    }

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * page_area_end() - Returns the end address of the page structure area.
 *
 * Return: A pointer to the end of the page structure area.
 */
void * page_area_end() {
    struct page * page_area = buddy_allocator.page_area;
    return (void*)((char*)page_area +
           (sizeof(struct page) * buddy_allocator.page_count));
}

/* ------------------------------------------------------------------------- */

/**
 * page_get_last() - Returns the last free page for a given order.
 * @allocator: The buddy allocator to retrieve the page from.
 * @order:     The order of the block to retrieve.
 *
 * Return: A pointer to the last free block, NULL if no blocks are available.
 */
struct page * page_get_last(struct buddy_allocator * allocator, uint32_t order) {
    list_head_t * head = &allocator->blocks[order].free_pages;

    /* Return NULL if no blocks are available */
    if(head->prev == NULL) {
        return NULL;
    }

    /* Return last block */
    return container_of(head->prev, struct page, buddy_node);
}

/* ------------------------------------------------------------------------- */

void page_print_debug(struct page * page) {
    klog("--- Page %d Info ---\n", page->pfn);
    klog("Use Count:  %d\n", page->use_count);
    klog("Flags:      0x%x\n", page->flags);
    klog("Order:      %d\n", page->order);
    klog("Phys. Addr: 0x%x\n", PAGE_PA(page));
    klog("Virt. Addr: 0x%x\n", PAGE_VA(page));
}

/* ------------------------------------------------------------------------- */

/**
 * buddy_init() - Initialises the page allocator.
 * @highest_pfn: The maximum page frame number to allocate page structs up to.
 *
 * Initialises a fixed array of page entries using memory allocated via
 * bootmem. Entries are created for all available physical addresses. Max
 * page number is retrieved from bootmem.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t buddy_init(uint32_t highest_pfn) {
    klog("buddy_init(): Initialising page allocator, %d "
                      "pages to initialise\n", highest_pfn);

    /* Buddy allocator initial set-up */
    klog("buddy_init(): Buddy allocator has max order "
                      "of %d\n", ORDER_MAX);
    memset(&buddy_allocator, 0, sizeof(struct buddy_allocator));
    memset(&blocks, 0, sizeof(blocks));
    buddy_allocator.max_order = ORDER_MAX;
    buddy_allocator.blocks = blocks;
    atomic_flag_clear(&buddy_allocator.lock);

    /* Calculate how many page structs we need to cover all avail. memory */
    page_count = highest_pfn;
    uint32_t bytes_req = page_count * sizeof(struct page);

    klog("buddy_init(): Setting up %d page structs, req. "
                      "%d bytes from bootmem allocator\n",
                      page_count, bytes_req);

    /* Calculate the (virtual) start and end address of the page area */
    buddy_allocator.page_area  = (struct page*)bootmem_alloc(bytes_req,
                                                        BM_NO_ALIGN);
    if(buddy_allocator.page_area == NULL) {
        PANIC("bootmem_alloc() returned NULL during allocation of the page "
              "struct area");
        return E_ERROR;
    }

    buddy_allocator.page_count = page_count;
    memset(buddy_allocator.page_area, 0, bytes_req);

    /* Initialise all pages, assigning them their zones and invalidating
     * known unusable regions */
    struct page * page = buddy_allocator.page_area;
    uintptr_t page_area_end = (uintptr_t)((char*)page +
                              (sizeof(struct page) * page_count));

    /* Initialise each buddy block list */
    for(uint32_t i = 0; i < ORDER_MAX + 1; i++) {
        klog("buddy_init(): Init. buddy allocator block list"
                          "for order 2^%d\n", i);
        clist_init(&buddy_allocator.blocks[i].free_pages);
        buddy_allocator.blocks[i].free_count = 0;
    }

    /* Initialise each page structure - we'll initially mark them all as
     * reserved and unusable, after which the bootmem subsystem will mark
     * pages as free based on information it has on available memory regions.
     * We'll also add LOWMEM, HIGHMEM and KERNEL flags as helpful metadata. */
    klog("buddy_init(): Initialising %d page structs, "
                      "region begins at 0x%x and ends at 0x%x\n", page_count,
                      page, page_area_end);

    for(uint32_t i = 0; i < page_count; i++) {
        /* Assign PFN and initial order */
        page->pfn = i;
        page->order = 0;

        /* Mark the page as INVALID - bootmem will free as appropriate later */
        SET_BIT(page->flags, PF_INVALID);

        /* Mark the page as high or low memory */
        if(PFN_TO_PA(page->pfn) < LOWMEM_PLIMIT) {
            SET_BIT(page->flags, PF_ZONE_LOWMEM);
            low_pages++;
        } else {
            SET_BIT(page->flags, PF_ZONE_HIGHMEM);
            high_pages++;
        }

        /* If the page contains kernel code or the page structs, mark it with
         * the KERNEL flag */
        uintptr_t page_phys_addr = PFN_TO_PA(page->pfn);
        if(page_phys_addr >= (uintptr_t)&KERNEL_PHYS_START &&
           page_phys_addr <  page_area_end) {
            SET_BIT(page->flags, PF_KERNEL);
        }

        /* Initialise the list node for the buddy allocator's use */
        clist_init(&page->buddy_node);

        page++;
    }

    klog("buddy_init(): Initialised %d pages (%d low, "
                      "%d high)\n", page_count, low_pages, high_pages);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * buddy_split_block() - Attempt to split a larger block into smaller blocks.
 * @order: The block order to be split into smaller blocks.
 *
 * Return: E_SUCCESS if the split was successful, E_ERROR if no
 *         block could be split.
 */
int32_t buddy_split_block(uint32_t order) {
    /* If we're asked to split the largest block possible, refuse */
    if(order > ORDER_MAX)
        return E_ERROR;

    /* We are normally asked to split blocks of an order 1 above the desired
     * block type, however there is no guarantee that blocks are free at this
     * order either. If this is the case, attempt to split higher again. */
    if(buddy_allocator.blocks[order].free_count == 0) {
        klog("No blocks at this order (%d) are free, attempting +1\n", order);
        buddy_split_block(order + 1);

        /* If free_count is still 0, it did not succeed */
        if(buddy_allocator.blocks[order].free_count == 0) {
            klog("Recursive split also failed!\n");
            return E_ERROR;
        }
    }

    /* Retrieve a page from the target order for us to split */
    struct page * target = page_get_last(&buddy_allocator, order);
    if(!target) {
        klog("Failed to retrieve a valid last block at order %d!\n", order);
        return E_ERROR;
    }
    klog("Splitting PFN %d, order %d\n", target->pfn, order);

    /* Remove the block from its order list */
    buddy_remove_block(target);

    /* Retrieve the buddy page */
    order--;
    struct page * buddy = buddy_get(target, order);

    buddy_add_block(target, order);
    buddy_add_block(buddy, order);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * buddy_merge_block() - Merge a block with its buddies to form larger blocks.
 * @block_page: Pointer to the structure representing the block to merge.
 * @order:      The order of the block being merged.
 *
 * Attempts to merge the specified block with its buddy blocks to create
 * larger free blocks, reducing fragmentation. The function continues merging
 * while the buddy blocks are free and of the same order, and the maximum
 * order has not been reached.
 *
 * Return: E_SUCCESS on success.
 */
int32_t buddy_merge_block(struct page * block_page, uint32_t order) {
    struct page * buddy = NULL;

    while(order < ORDER_MAX) {
        buddy = buddy_get(block_page, order);

        /* Ensure the buddy is eligible to be merged (is free and valid) */
        if(buddy->order != order) {
            break;
        }

        /* If the buddy page is INVALID, stop - this is important as a buddy
         * page may fall within reserved or invalid memory regions */
        if(TEST_BIT(buddy->flags, PF_INVALID)) {
            break;
        }

        /* This block is getting merged, so remove it from its current list */
        clist_delete_node(&buddy->buddy_node);

        /* Decrement the available free page count for the order */
        buddy_allocator.blocks[order].free_count--;

        /* Make sure we're working with the lowest page of the pair */
        block_page = page_from_pfn(block_page->pfn & ~(1 << order));

        /* Get its higher buddy, and set it to the relevant order */
        struct page * p = buddy_get(block_page, order);
        p->order = order;

        order++;
    }

    /* Finally, set the merged block's order and add it to the lists */
    buddy_add_block(block_page, order);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * buddy_get() - Retrieve the buddy page of a page.
 *
 * Return: A pointer to the buddy page of the specified page.
 */
struct page * buddy_get(struct page * page, uint32_t order) {
    uint32_t buddy_pfn = page->pfn ^ (1 << order);
    return page_from_pfn(buddy_pfn);
}

/* ------------------------------------------------------------------------- */

/**
 * buddy_remove_block() - Remove a block from the buddy allocator's free list.
 * @block_page: Pointer to the structure representing the block to remove.
 *
 * Removes the specified block from its free list in the buddy allocator and
 * updates the free count.
 */
void buddy_remove_block(struct page * block_page) {
    clist_delete_node(&block_page->buddy_node);
    buddy_allocator.blocks[block_page->order].free_count--;
    block_page->order = ORDER_USED;
}

/* ------------------------------------------------------------------------- */

/**
 * buddy_add_block() - Add a block to the buddy allocator's free list.
 * @block_page: Pointer to the page structure representing the block to add.
 * @order:      The order of the block being added.
 *
 * Adds the specified block to the buddy allocator's free list at the given
 * order and updates the free count.
 */
void buddy_add_block(struct page * block_page, uint32_t order) {
    block_page->order = order;
    clist_add(&buddy_allocator.blocks[order].free_pages,
              &block_page->buddy_node);
    buddy_allocator.blocks[order].free_count++;
}

/* ------------------------------------------------------------------------- */

/**
 * buddy_print_debug() - Print the current state of the buddy allocator.
 *
 * Outputs debugging information about the buddy allocator, including page
 * counts and free and used counts for each order, to the serial console.
 */
void buddy_print_debug() {
    klog("--- Buddy Allocator Info ---\n");
    klog("Page Count:          %d\n", buddy_allocator.page_count);
    klog("Max Order:           %d\n", ORDER_MAX);
    klog("Page Area Start:     0x%x\n",
                      buddy_allocator.page_area);
    klog("Page Area End:       0x%x\n",
                      buddy_allocator.page_area +
                      (buddy_allocator.page_count * sizeof(struct page)));

    for(uint32_t i = 0; i < ORDER_MAX + 1; i++) {
        klog("Order[%d] Free: %d\n", i,
             buddy_allocator.blocks[i].free_count);
    }
}

/* ------------------------------------------------------------------------- */

/* Include unit tests */
#include "test/palloc.c"

/* ------------------------------------------------------------------------- */
