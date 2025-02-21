/*
 * kernel/test/palloc.c
 * Page Frame Allocator Testing
 */

#include <rotary/test/palloc.h>

extern uintptr_t KERNEL_PHYS_START;
extern uintptr_t KERNEL_PHYS_END;

/* ------------------------------------------------------------------------- */
/* Test Set-up and Clean-up                                                  */
/* ------------------------------------------------------------------------- */

int32_t palloc_pre_module(ktest_module_t * module) {
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

int32_t palloc_post_module(ktest_module_t * module) {
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

int32_t palloc_pre_test(ktest_module_t * module) {
    /* Page allocator initialisation requests memory from bootmem to store
     * its page structures, therefore we will want to ensure that bootmem
     * is in a consistent state each time we re-initialise palloc */
    bootmem_reset();

    /* Ensure the buddy allocator state is completely reset */
    memset(&buddy_allocator, 0, sizeof(struct buddy_allocator));
    memset(&blocks, 0, sizeof(struct block_list));
    high_pages = 0;
    low_pages  = 0;

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

int32_t palloc_post_test(ktest_module_t * module) {
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */
/* Utility Functions                                                         */
/* ------------------------------------------------------------------------- */

void palloc_test_configure_memory(uintptr_t start_addr,
                                  uintptr_t end_addr) {
    /* We set up a temporary bootmem region which will be subsequently used
     * by buddy_init() to get space for the page struct pool. For the sake
     * of convenience, we will place the pool at a separate address.
     *
     * Where we define this region _does_ matter, because the struct page struct
     * pool will actually go into real memory at that location. Later on, we
     * can use a 'fictional' region when testing the buddy allocator. */
    uint32_t  page_count = end_addr / PAGE_SIZE;
    uintptr_t pool_start = 0x200000;
    uintptr_t pool_end   = pool_start + (sizeof(struct page) * page_count);
    bootmem_add_mem_region(pool_start, pool_end, MEM_REGION_AVAILABLE);

    /* We initialise the buddy allocator, which will request memory from
     * bootmem for the page struct pool. All pages will be configured,
     * but marked as unusable. */
    uint32_t highest_pfn = end_addr / PAGE_SIZE;
    buddy_init(highest_pfn);

    /* To make testing more predictable, we reset the bootmem allocator and
     * create a new usable memory region far away from the memory used to
     * store the page structs. If we don't do this, bootmem_mark_free() will
     * attempt to split the remaining formerly defined memory region up into
     * blocks of 2^0, 2^1, 2^2 etc. pages.
     *
     * Because the page struct pool wil have eaten into our memory region a bit
     * already, we will likely end up with an irregular variety of block sizes,
     * as it's very unlikely that the remainder of the memory space can be
     * perfectly filled with blocks of the highest order. We would likely end
     * up with something like 53 blocks of size 2^5, one block of 2^4, one of
     * 2^3, zero of 2^2 and so on. This is hard to write expectations for!
     *
     * By adding a new, fresh region of memory and removing the old, bootmem
     * and the buddy allocator will hopefully be able to produce a predictable
     * set of higher order blocks that can be assert()'d for as we can predict
     * this number based on the address range we provide. */
    bootmem_reset();
    bootmem_add_mem_region(start_addr, end_addr, MEM_REGION_AVAILABLE);

    /* Ask the bootmem subsystem to mark available pages */
    bootmem_mark_free();
}

/* ------------------------------------------------------------------------- */
/* Unit Tests                                                                */
/* ------------------------------------------------------------------------- */

void palloc_test_buddy_init(ktest_unit_t * ktest) {
    /* Register 128MB of usable memory */
    uintptr_t start_addr = 0x200000;
    uintptr_t end_addr = 0x8200000;
    uint32_t  highest_pfn = end_addr / PAGE_SIZE;
    bootmem_add_mem_region(start_addr, end_addr, MEM_REGION_AVAILABLE);

    /* Initialise the buddy allocator */
    int rv = buddy_init(bootmem_highest_pfn());

    /* Validate init return value and attributes of the buddy allocator */
    assert_equal(rv, E_SUCCESS);
    assert_equal(page_count, end_addr / PAGE_SIZE);
    assert_equal(buddy_allocator.max_order, ORDER_MAX);
    assert_equal(buddy_allocator.page_area, PHY_TO_VIR(start_addr));
    assert_equal(buddy_allocator.page_count, highest_pfn);
    assert_equal(buddy_allocator.blocks, &blocks);
    assert_equal(sizeof(blocks), (ORDER_MAX+1) * sizeof(struct block_list));

    /* Validate correct initial settings for each buddy block list */
    for(uint32_t i = 0; i < ORDER_MAX+1; i++) {
        assert_equal(buddy_allocator.blocks[i].free_count, 0);
        assert_equal(buddy_allocator.blocks[i].used_count, 0);
    }

    /* Validate correct initial setting of each page structure */
    struct page * page = buddy_allocator.page_area;
    uint32_t expected_high = 0;
    uint32_t expected_low  = 0;
    for(uint32_t i = 0; i < highest_pfn; i++) {
        assert_equal(page->pfn, i);
        assert_equal(page->order, 0);
        assert_bit_set(page->flags, PF_INVALID);
        if(PFN_TO_PA(page->pfn) < LOWMEM_PLIMIT) {
            assert_bit_set(page->flags, PF_ZONE_LOWMEM);
            expected_low++;
        } else {
            assert_bit_set(page->flags, PF_ZONE_HIGHMEM);
            expected_high++;
        }
        page++;
    }

    assert_equal(high_pages, expected_high);
    assert_equal(low_pages, expected_low);
}

/* ------------------------------------------------------------------------- */

void palloc_test_alloc_min_order(ktest_unit_t * ktest) {
    /* Configure 128MB of usable memory */
    palloc_test_configure_memory(0x400000, 0x8400000);

    /* Keep a record of how many highest order blocks exist after buddy
     * allocator initialisation, so we can check it decreases and increases
     * appropriately as we allocate and de-allocate */
    uint32_t max_blocks = buddy_allocator.blocks[ORDER_MAX].free_count;

    /* Request a page allocation of a single page (order 2^0) */
    struct page * page = page_alloc(0, 0);

    /* Ensure allocation succeeded, and that we now see one less highest order
     * block available */
    assert_not_equal(page, NULL);
    if(!page) return;
    assert_equal(buddy_allocator.blocks[ORDER_MAX].free_count,
                 max_blocks - 1);

    /* Validate expected attributes of the returned page */
    assert_equal(page->order, ORDER_USED);

    /* As we have a memory range that allows for perfect initial splitting
     * into the highest order blocks, we can assume that there will be 1 of
     * each order block all the way up but not including the max order, as
     * a highest order block will have been split recursively down to create
     * our minimum order block allocation for this test */
    for(uint32_t i = 0; i < ORDER_MAX; i++) {
        assert_equal(buddy_allocator.blocks[i].free_count, 1);
    }

    /* Free the page - given our memory range and lack of other allocations,
     * we can expect this page to be full merged back into a highest order
     * block, with all of the lower order blocks merged back together */
    int rv = page_free(page, 0);
    assert_equal(rv, E_SUCCESS);
    assert_equal(buddy_allocator.blocks[ORDER_MAX].free_count,
                 max_blocks);
    for(uint32_t i = 0; i < ORDER_MAX; i++) {
        assert_equal(buddy_allocator.blocks[i].free_count, 0);
    }
}

/* ------------------------------------------------------------------------- */

void palloc_test_alloc_max_order(ktest_unit_t * ktest) {
    /* Configure 128MB of usable memory */
    palloc_test_configure_memory(0x400000, 0x8400000);

    uint32_t max_blocks = buddy_allocator.blocks[ORDER_MAX].free_count;

    /* Request a page allocation of a highest order block */
    struct page * page = page_alloc(ORDER_MAX, 0);

    /* Ensure allocation succeeded, and that we now see one less highest order
     * block available */
    assert_not_equal(page, NULL);
    assert_equal(buddy_allocator.blocks[ORDER_MAX].free_count,
                 max_blocks - 1);

    /* Validate expected attributes of the returned page */
    assert_equal(page->order, ORDER_USED);

    /* Ensure we have no smaller blocks available */
    for(uint32_t i = 0; i < ORDER_MAX; i++) {
        assert_equal(buddy_allocator.blocks[i].free_count, 0);
    }

    /* Free the highest order block and verify success */
    int rv = page_free(page, ORDER_MAX);
    assert_equal(rv, E_SUCCESS);

    /* Validate that the number of highest order blocks has increased by 1 */
    assert_equal(buddy_allocator.blocks[ORDER_MAX].free_count, max_blocks);

    /* Ensure we have no smaller blocks available, post-free */
    for(uint32_t i = 0; i < ORDER_MAX; i++) {
        assert_equal(buddy_allocator.blocks[i].free_count, 0);
    }
}

/* ------------------------------------------------------------------------- */

void palloc_test_alloc_exhaust_all(ktest_unit_t * ktest) {
    /* Configure 128MB of usable memory */
    palloc_test_configure_memory(0x400000, 0x8400000);

    /* Allocate all available highest order blocks, then try and allocate again
     * once they're exhausted and verify failure */
    int max_count = buddy_allocator.blocks[ORDER_MAX].free_count;
    struct page * page = NULL;
    for(uint32_t i = 0; i < max_count; i++) {
        page = page_alloc(ORDER_MAX, 0);
        assert_not_equal(page, NULL);
    }

    for(uint32_t i = 0; i < ORDER_MAX+1; i++) {
        int orig_count = buddy_allocator.blocks[i].free_count;
        page = page_alloc(i, 0);
        assert_equal(page, NULL);
        assert_equal(buddy_allocator.blocks[i].free_count, orig_count);
    }
}

/* ------------------------------------------------------------------------- */

void palloc_test_alloc_split_and_free(ktest_unit_t * ktest) {
    /* Configure 128MB of usable memory */
    palloc_test_configure_memory(0x400000, 0x8400000);

    /* Store block state prior to allocation */
    int orig_max_count   = buddy_allocator.blocks[ORDER_MAX].free_count;

    /* Attempt to allocate a block of ORDER_MAX-1, of which no blocks should
     * currently exist for */
    struct page * page1 = page_alloc(ORDER_MAX - 1, 0);

    /* It should succeed after splitting an ORDER_MAX block */
    assert_not_equal(page1, NULL);
    assert_equal(buddy_allocator.blocks[ORDER_MAX].free_count,
                 orig_max_count - 1);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-1].free_count, 1);

    /* Now we should have an OM-1 block free, as it is the buddy to the block
     * that was allocated for us. Let's now split that, and check that OM-1
     * blocks becomes zero. */
    struct page * page2 = page_alloc(ORDER_MAX - 2, 0);
    assert_not_equal(page2, NULL);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-1].free_count, 0);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-2].free_count, 1);

    /* Finally, let's free our blocks and verify that the block list returns
     * to an expected state - the first page should just become available
     * and no merging should occur, because its buddy is in use by our
     * second allocation */
    int rv = page_free(page1, ORDER_MAX - 1);
    assert_equal(rv, E_SUCCESS);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-1].free_count, 1);

    /* Our OM-2 allocation should now cause a merge that will restore all
     * blocks to highest order blocks, as no buddies are in use */
    rv = page_free(page2, ORDER_MAX - 2);
    assert_equal(rv, E_SUCCESS);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-2].free_count, 0);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-1].free_count, 0);
    assert_equal(buddy_allocator.blocks[ORDER_MAX].free_count, orig_max_count);
}

/* ------------------------------------------------------------------------- */

void palloc_test_free_null(ktest_unit_t * ktest) {
    /* Configure 128MB of usable memory */
    palloc_test_configure_memory(0x400000, 0x8400000);

    /* Attempt to free a NULL pointer */
    int rv = page_free(NULL, 0);
    assert_equal(rv, E_ERROR);
}

/* ------------------------------------------------------------------------- */

void palloc_test_free_critical(ktest_unit_t * ktest) {
    /* Configure 128MB of usable memory */
    palloc_test_configure_memory(0x400000, 0x8400000);

    /* Get a handle to a kernel page */
    struct page * page = page_from_pfn(
        PA_TO_PFN((uintptr_t)&KERNEL_PHYS_START)
    );
    assert_not_equal(page, NULL);

    /* Store how many pages of the same order exist prior to free attempt */
    int count_pre = buddy_allocator.blocks[page->order].free_count;

    /* Attempt to free the page, it should fail */
    int rv = page_free(page, page->order);
    assert_equal(rv, E_ERROR);

    /* Make sure that the block list has remained unaffected */
    assert_equal(buddy_allocator.blocks[page->order].free_count,
                 count_pre);
}

/* ------------------------------------------------------------------------- */

void palloc_test_is_critical(ktest_unit_t * ktest) {
    /* Configure 128MB of usable memory */
    palloc_test_configure_memory(0x400000, 0x8400000);

    /* Get a handle to a kernel page */
    struct page * page = page_from_pfn(
        PA_TO_PFN((uintptr_t)&KERNEL_PHYS_START)
    );
    assert_not_equal(page, NULL);

    /* Check if the page is appropriately detected as critical */
    int rv = page_is_critical(page);
    assert_equal(rv, E_ERROR);
}

/* ------------------------------------------------------------------------- */

void palloc_test_partial_block_free(ktest_unit_t * ktest) {
    /* Configure 128MB of usable memory */
    palloc_test_configure_memory(0x400000, 0x8400000);

    /* Allocate two order 0 pages from the same order 1 block */
    struct page *page1 = page_alloc(0, 0);
    struct page *page2 = page_alloc(0, 0);

    /* Check that we have an expected number of blocks in the allocator - this
     * assumes that 128MB of memory is used on a 4KB page system */
    assert_equal(buddy_allocator.blocks[ORDER_MAX].free_count, 511);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-1].free_count, 1);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-2].free_count, 1);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-3].free_count, 1);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-4].free_count, 1);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-5].free_count, 1);
    assert_equal(buddy_allocator.blocks[ORDER_MAX-6].free_count, 0);

    /* Ensure both allocations succeeded */
    assert_not_equal(page1, NULL);
    assert_not_equal(page2, NULL);
    if (!page1 || !page2) return;

    /* Verify both pages have correct order */
    assert_equal(page1->order, ORDER_USED);
    assert_equal(page2->order, ORDER_USED);

    /* Free the first page */
    int rv1 = page_free(page1, 0);
    assert_equal(rv1, E_SUCCESS);

    /* Verify that the first page's order is reset appropriately */
    assert_equal(page1->order, 0);

    /* Verify that the second page's order is still marked as used */
    assert_equal(page2->order, ORDER_USED);

    /* Free the second page to clean up */
    int rv2 = page_free(page2, 0);
    assert_equal(rv2, E_SUCCESS);
    assert_equal(page2->order, 0);
}

/* ------------------------------------------------------------------------- */
/* Test Registration                                                         */
/* ------------------------------------------------------------------------- */

static ktest_unit_t test_units[] = {
    KTEST_UNIT("palloc-buddy-init", palloc_test_buddy_init),
    KTEST_UNIT("palloc-test-min-order", palloc_test_alloc_min_order),
    KTEST_UNIT("palloc-test-max-order", palloc_test_alloc_max_order),
    KTEST_UNIT("palloc-test-alloc-exhaust-all", palloc_test_alloc_exhaust_all),
    KTEST_UNIT("palloc-test-alloc-split-and-free",
               palloc_test_alloc_split_and_free),
    KTEST_UNIT("palloc-test-free-null", palloc_test_free_null),
    KTEST_UNIT("palloc-test-free-critical", palloc_test_free_critical),
    KTEST_UNIT("palloc-test-is-critical", palloc_test_is_critical),
    KTEST_UNIT("palloc-test-partial-block-free",
               palloc_test_partial_block_free),
};

KTEST_MODULE_DEFINE("palloc", test_units,
                    palloc_pre_module,
                    palloc_post_module,
                    palloc_pre_test,
                    palloc_post_test);

/* ------------------------------------------------------------------------- */
