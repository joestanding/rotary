/*
 * kernel/test/bootmem.c
 * Bootmem Allocator Testing
 */

#include <rotary/test/bootmem.h>

/* ------------------------------------------------------------------------- */
/* Test Set-up and Clean-up                                                  */
/* ------------------------------------------------------------------------- */

int32_t bootmem_pre_module(ktest_module_t * module) {
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

int32_t bootmem_post_module(ktest_module_t * module) {
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

int32_t bootmem_pre_test(ktest_module_t * module) {
    /* Clear the memory regions array */
    memset(mem_regions, 0, sizeof(mem_regions));

    /* Reset highest observed PFN and region count */
    highest_pfn = 0;
    region_count = 0;

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

int32_t bootmem_post_test(ktest_module_t * module) {
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */
/* Unit Tests                                                                */
/* ------------------------------------------------------------------------- */

void bootmem_test_add_all_regions(ktest_unit_t * ktest) {
    for(uint32_t i = 0; i < MAX_MEM_REGIONS; i++) {
        uintptr_t start_addr = 0x10000 + (i * PAGE_SIZE);
        uintptr_t end_addr   = 0x11000 + (i * PAGE_SIZE);
        int rv = bootmem_add_mem_region(start_addr, end_addr,
                                        MEM_REGION_AVAILABLE);
        assert_equal(rv, E_SUCCESS);
        assert_equal(region_count, i + 1);
        assert_equal(highest_pfn, end_addr / PAGE_SIZE);
    }
}

/* ------------------------------------------------------------------------- */

void bootmem_test_excess_regions(ktest_unit_t * ktest) {
    for(uint32_t i = 0; i < MAX_MEM_REGIONS + 1; i++) {
        uintptr_t start_addr = 0x10000 + (i * PAGE_SIZE);
        uintptr_t end_addr   = 0x11000 + (i * PAGE_SIZE);
        int rv = bootmem_add_mem_region(start_addr, end_addr,
                                        MEM_REGION_AVAILABLE);
        if(i == MAX_MEM_REGIONS) {
            assert_equal(rv, E_ERROR);
            assert_equal(region_count, i);
            assert_equal(highest_pfn, (end_addr - PAGE_SIZE) / PAGE_SIZE);
        } else {
            assert_equal(rv, E_SUCCESS);
            assert_equal(region_count, i + 1);
            assert_equal(highest_pfn, end_addr / PAGE_SIZE);
        }
    }
}

/* ------------------------------------------------------------------------- */

void bootmem_test_kernel_region(ktest_unit_t * ktest) {
    uintptr_t kernel_start = PAGE_ALIGN(&KERNEL_PHYS_START);
    uintptr_t kernel_end   = PAGE_ALIGN(&KERNEL_PHYS_END);
    kernel_end -= PAGE_SIZE;
    int rv = bootmem_add_mem_region(kernel_start, kernel_end,
                                    MEM_REGION_AVAILABLE);
    assert_equal(rv, E_ERROR);
    assert_equal(region_count, 0);
    assert_equal(highest_pfn, 0);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_invalid_type(ktest_unit_t * ktest) {
    int rv = bootmem_add_mem_region(0x10000, 0x11000, MEM_REGION_AVAILABLE);
    assert_equal(rv, E_SUCCESS);
    assert_equal(region_count, 1);
    assert_equal(highest_pfn, 0x11000 / PAGE_SIZE);
    rv = bootmem_add_mem_region(0x20000, 0x21000, MEM_REGION_RESERVED);
    assert_equal(rv, E_SUCCESS);
    assert_equal(region_count, 2);
    assert_equal(highest_pfn, 0x21000 / PAGE_SIZE);
    rv = bootmem_add_mem_region(0x30000, 0x31000, 2);
    assert_equal(rv, E_ERROR);
    assert_equal(region_count, 2);
    assert_equal(highest_pfn, 0x21000 / PAGE_SIZE);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_highest_pfn(ktest_unit_t * ktest) {
    uintptr_t end_addr = 0x11000;
    int rv = bootmem_add_mem_region(0x10000, end_addr, MEM_REGION_AVAILABLE);
    assert_equal(rv, E_SUCCESS);
    assert_equal(region_count, 1);
    assert_equal(highest_pfn, end_addr / PAGE_SIZE);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_koverlap_low(ktest_unit_t * ktest) {
    uintptr_t kernel_start = PAGE_ALIGN(&KERNEL_PHYS_START);

    /* One page before kernel start -> one page after kernel start */
    uintptr_t start_addr   = kernel_start - PAGE_SIZE;
    uintptr_t end_addr     = kernel_start + PAGE_SIZE;

    bootmem_add_mem_region(start_addr, end_addr, MEM_REGION_AVAILABLE);

    /* Expect start preserved, but end adjusted to kernel start */
    assert_equal(region_count, 1);
    assert_equal(highest_pfn, kernel_start / PAGE_SIZE);
    assert_equal(mem_regions[0].start_addr, start_addr);
    assert_equal(mem_regions[0].end_addr, kernel_start);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_koverlap_high(ktest_unit_t * ktest) {
    uintptr_t kernel_end   = PAGE_ALIGN(&KERNEL_PHYS_END);

    /* One page before kernel end -> one page after kernel end */
    uintptr_t start_addr   = kernel_end - PAGE_SIZE;
    uintptr_t end_addr     = kernel_end + PAGE_SIZE;

    bootmem_add_mem_region(start_addr, end_addr, MEM_REGION_AVAILABLE);

    /* Expect start adjusted to kernel end, but end preserved */
    assert_equal(region_count, 1);
    assert_equal(highest_pfn, end_addr / PAGE_SIZE);
    assert_equal(mem_regions[0].start_addr, kernel_end);
    assert_equal(mem_regions[0].end_addr, end_addr);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_koverlap_all(ktest_unit_t * ktest) {
    uintptr_t kernel_start = PAGE_ALIGN(&KERNEL_PHYS_START);
    uintptr_t kernel_end   = PAGE_ALIGN(&KERNEL_PHYS_END);

    /* One page before kernel start -> one page after kernel end */
    uintptr_t start_addr   = kernel_start - PAGE_SIZE;
    uintptr_t end_addr     = kernel_end + PAGE_SIZE;

    bootmem_add_mem_region(start_addr, end_addr, MEM_REGION_AVAILABLE);

    /* Expect two regions:
     * Region 1: End amended to kernel start
     * Region 2: Start amended to kernel end
     */
    assert_equal(region_count, 2);
    assert_equal(highest_pfn, end_addr / PAGE_SIZE);
    assert_equal(mem_regions[0].start_addr, start_addr);
    assert_equal(mem_regions[0].end_addr, kernel_start);
    assert_equal(mem_regions[1].start_addr, kernel_end);
    assert_equal(mem_regions[1].end_addr, end_addr);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_alloc_ok_partial(ktest_unit_t * ktest) {
    uintptr_t start_addr = 0x10000;
    uintptr_t end_addr   = start_addr + (PAGE_SIZE * 4);
    bootmem_add_mem_region(start_addr, end_addr, MEM_REGION_AVAILABLE);
    void * rv = bootmem_alloc(PAGE_SIZE, BM_NO_ALIGN);
    assert_not_equal(rv, NULL);
    assert_equal(rv, PHY_TO_VIR(start_addr));
}

/* ------------------------------------------------------------------------- */

void bootmem_test_alloc_ok_all(ktest_unit_t * ktest) {
    uintptr_t start_addr = 0x10000;
    uintptr_t end_addr   = start_addr + (PAGE_SIZE * 4);
    bootmem_add_mem_region(start_addr, end_addr, MEM_REGION_AVAILABLE);
    void * rv = bootmem_alloc(PAGE_SIZE * 4, BM_NO_ALIGN);
    assert_not_equal(rv, NULL);
    assert_equal(rv, PHY_TO_VIR(start_addr));
}

/* ------------------------------------------------------------------------- */

void bootmem_test_alloc_bad_exceed(ktest_unit_t * ktest) {
    uintptr_t start_addr = 0x10000;
    uintptr_t end_addr   = start_addr + (PAGE_SIZE * 4);
    bootmem_add_mem_region(start_addr, end_addr, MEM_REGION_AVAILABLE);
    void * rv = bootmem_alloc(PAGE_SIZE * 5, BM_NO_ALIGN);
    assert_equal(rv, NULL);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_alloc_bad_no_regions(ktest_unit_t * ktest) {
    void * rv = bootmem_alloc(PAGE_SIZE, BM_NO_ALIGN);
    assert_equal(rv, NULL);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_alloc_multiple_regions(ktest_unit_t * ktest) {
    /* Create region 1, two pages in size */
    uintptr_t start_addr1 = 0x10000;
    uintptr_t end_addr1   = start_addr1 + (PAGE_SIZE * 2);
    bootmem_add_mem_region(start_addr1, end_addr1, MEM_REGION_AVAILABLE);

    /* Create region 2, two pages in size */
    uintptr_t start_addr2 = 0x20000;
    uintptr_t end_addr2   = start_addr2 + (PAGE_SIZE * 2);
    bootmem_add_mem_region(start_addr2, end_addr2, MEM_REGION_AVAILABLE);

    /* Expect allocation from region 1 */
    void * rv = bootmem_alloc(PAGE_SIZE * 2, BM_NO_ALIGN);
    assert_equal(rv, PHY_TO_VIR(start_addr1));

    /* Expect allocation from region 2 */
    rv = bootmem_alloc(PAGE_SIZE * 2, BM_NO_ALIGN);
    assert_equal(rv, PHY_TO_VIR(start_addr2));

    /* Expect failure */
    rv = bootmem_alloc(PAGE_SIZE, BM_NO_ALIGN);
    assert_equal(rv, NULL);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_zero_length_region(ktest_unit_t * ktest) {
    uintptr_t start_addr = 0x10000;
    uintptr_t end_addr   = 0x10000;
    int rv = bootmem_add_mem_region(start_addr, end_addr,
                                    MEM_REGION_AVAILABLE);
    assert_equal(rv, E_ERROR);
    assert_equal(region_count, 0);
    assert_equal(highest_pfn, 0);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_invalid_region_bounds(ktest_unit_t * ktest) {
    uintptr_t start_addr = 0x11000;
    uintptr_t end_addr   = 0x10000;
    int rv = bootmem_add_mem_region(start_addr, end_addr,
                                    MEM_REGION_AVAILABLE);
    assert_equal(rv, E_ERROR);
    assert_equal(region_count, 0);
    assert_equal(highest_pfn, 0);
}

/* ------------------------------------------------------------------------- */

void bootmem_test_reset(ktest_unit_t * ktest) {
    bootmem_add_mem_region(0x10000, 0x11000, MEM_REGION_AVAILABLE);
    bootmem_reset();
    assert_clear(mem_regions, sizeof(mem_regions));
    assert_equal(region_count, 0);
    assert_equal(highest_pfn, 0);
}

/* ------------------------------------------------------------------------- */
/* Test Registration                                                         */
/* ------------------------------------------------------------------------- */

static ktest_unit_t test_units[] = {
    KTEST_UNIT("bootmem-test-add-all-regions", bootmem_test_add_all_regions),
    KTEST_UNIT("bootmem-test-excess-regions", bootmem_test_excess_regions),
    KTEST_UNIT("bootmem-test-kernel-region", bootmem_test_kernel_region),
    KTEST_UNIT("bootmem-test-invalid-type", bootmem_test_invalid_type),
    KTEST_UNIT("bootmem-test-highest-pfn", bootmem_test_highest_pfn),
    KTEST_UNIT("bootmem-test-koverlap-low", bootmem_test_koverlap_low),
    KTEST_UNIT("bootmem-test-koverlap-high", bootmem_test_koverlap_high),
    KTEST_UNIT("bootmem-test-koverlap-all", bootmem_test_koverlap_all),
    KTEST_UNIT("bootmem-test-alloc-ok-partial", bootmem_test_alloc_ok_partial),
    KTEST_UNIT("bootmem-test-alloc-ok-all", bootmem_test_alloc_ok_all),
    KTEST_UNIT("bootmem-test-alloc-bad-exceed", bootmem_test_alloc_bad_exceed),
    KTEST_UNIT("bootmem-test-reset", bootmem_test_reset),
    KTEST_UNIT("bootmem-test-alloc-bad-no-regions",
               bootmem_test_alloc_bad_no_regions),
    KTEST_UNIT("bootmem-test-alloc-multiple-regions",
               bootmem_test_alloc_multiple_regions),
    KTEST_UNIT("bootmem-test-zero-length-region",
               bootmem_test_zero_length_region),
    KTEST_UNIT("bootmem-test-invalid-region-bounds",
               bootmem_test_invalid_region_bounds),
};

KTEST_MODULE_DEFINE("bootmem", test_units,
                    bootmem_pre_module,
                    bootmem_post_module,
                    bootmem_pre_test,
                    bootmem_post_test);

/* ------------------------------------------------------------------------- */
