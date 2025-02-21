/**
 * kernel/mm/ptable.c
 * Arch-independent Page Table Operations
 *
 * Functions to update and manipulate page tables are implemented here in an
 * architecture-independent manner. The functions make use of macros and data
 * structures defined by each architecture implementation, such as pte and
 * MAKE_PTE(). Each architecture implements these structures and macros in ways
 * appropriate for their platform, so that they can be used in a platform
 * neutral way.
 *
 * Implementing page table operations in this manner allows the same logic to
 * be used across platforms, while architecture-specific differences can be
 * managed by the macros and structures defined by the architecture.
 *
 * Architecture-specific implementations can generally be found in
 * arch/<arch>/include/arch/ptable.h and paging.h.
 */

#include <rotary/mm/ptable.h>

/* ------------------------------------------------------------------------- */

/**
 * ptable_pgd_new() - Allocate a new top-level page global directory.
 *
 * Allocates memory for a new top-level page global directory, ensures the
 * memory is cleared, then copies the kernel page directory into it. This
 * ensures that all new page tables created for tasks contain the essential
 * kernel mappings. There is no scenario where we want to create a page table
 * without the kernel mappings.
 *
 * Return: A pointer to the struct pgd object for the new page directory.
 */
struct pgd * ptable_pgd_new() {
    struct pgd * pgd = page_alloc_va(0, PR_KERNEL);
    if(!pgd)
        return NULL;

    memset(pgd, 0, PAGE_SIZE);
    memcpy(pgd, paging_kernel_pgd(), PAGE_SIZE);

    return pgd;
}

/* ------------------------------------------------------------------------- */

/**
 * ptable_pgd_free() - Frees and erases a top-level page global directory.
 * @pgd: A pointer to the virtual address of the PGD to be freed.
 *
 * Frees the page used for the PGD, and frees pages for any page tables that
 * do not map kernel memory using ptable_unmap() and ptable_unmap_many().
 */
void ptable_pgd_free(struct pgd * pgd) {
    klog("ptable_pgd_free(): Freeing PGD at 0x%x\n", pgd);

    /* Free all page tables that don't cover kernel space */
    int pde_max = PAGE_DIRECTORY_INDEX(KERNEL_START_VIRT);
    for(int pde_index = 0; pde_index < pde_max; pde_index++) {
        struct pde * pde = &pgd->entries[pde_index];
        if(!PDE_EXISTS(pde)) {
            continue;
        }

        if(PDE_IS_HUGE(pde)) {
            /* TODO: free 4MB page area */
        } else {
            struct pgt * pgt = PDE_TO_PGT(&pgd->entries[pde_index]);

            for(int pte_index = 0; pte_index < PAGE_TABLE_SIZE; pte_index++) {
                struct pte * pte = &pgt->entries[pte_index];
                if(!PTE_EXISTS(pte))
                    continue;
                page_free_va(PTE_VA(pte), 0);
            }
        }

        /* De-allocate the PDE */
        page_free_va(PDE_VA(pde), 0);
    }

    /* Finally, de-allocate the PGD itself */
    page_free_va(pgd, 0);
}

/* ------------------------------------------------------------------------- */

/**
 * ptable_map() - Add a single page mapping to a page table.
 * @pgd:       The top-level page table (PGD) to add the mapping to.
 * @virt_addr: The virtual address to map from.
 * @phys_addr: The physical address to map to.
 * @flags:     Flags for the page (e.g. writable)
 *
 * Maps a virtual address to a physical address in the specified page table.
 * Allocates and initializes intermediate page table structures if needed.
 * Configures the mapping based on the provided flags.
 *
 * Valid flags are VM_MAP_*, as ptable_map() will nearly always be invoked
 * by code processing a vm_map.
 */
void ptable_map(struct pgd * pgd, void * virt_addr, void * phys_addr,
                flags_t flags) {
    /* Make the page table entry first, then add flags if necessary later */
    struct pte entry = MAKE_PTE(phys_addr, PTE_PRESENT | PTE_USER);

    /* Make the page writable if specified */
    if(TEST_BIT(flags, VM_MAP_WRITE)) {
        PTE_SET_WRITABLE(&entry);
    }

    /* Get the top-level page directory entry responsible for this address */
    struct pde * pde = GET_PDE(pgd, virt_addr);

    /* If one doesn't already exist, allocate memory for one and assign it */
    if(!PDE_EXISTS(pde)) {
        klog("ptable_map(): PDE for vaddr 0x%x does not exist\n", virt_addr);
        struct page * page = page_alloc(0, PR_KERNEL);
        memset(PAGE_VA(page), 0, PAGE_SIZE);

        /* Point the PDE entry to our newly allocated page table */
        *pde = MAKE_PDE(PAGE_PA(page), PDE_PRESENT |
                                          PDE_WRITABLE |
                                          PDE_USER);
    }

    /* Get a pointer to the page table itself using the address in the PDE */
    struct pgt * pgt = PDE_TO_PGT(pde);

    /* Get a pointer to the entry within the page table */
    struct pte * pte = GET_PTE(pgt, virt_addr);
    *pte = entry;
}

/* ------------------------------------------------------------------------- */

/**
 * ptable_map_many() - Add multiple contiguous page mappings to a page table.
 * @pgd:       The top-level page table (PGD) to add the mapping to.
 * @virt_addr: The virtual address to map from.
 * @phys_addr: The physical address to map to.
 * @count:     How many pages to map.
 * @flags:     Flags for the page (e.g. writable)
 *
 * Adds multiple contiguous page mappings to a table by calling ptable_map()
 * for each page address in the range.
 */
void ptable_map_many(struct pgd * pgd, void * virt_addr, void * phys_addr,
                     int count, flags_t flags) {
    for(uint32_t i = 0; i < count; i++) {
        ptable_map(pgd, virt_addr + i * PAGE_SIZE, phys_addr + i * PAGE_SIZE,
                   flags);
    }
}

/* ------------------------------------------------------------------------- */

/**
 * ptable_unmap() - Remove a single page mapping from a page table.
 * @pgd:       The top-level page table (PGD) to remove the mapping from.
 * @virt_addr: The virtual address to unmap from.
 * @free:      Whether any pages allocated for the address should be freed.
 *
 * Removes a single page mapping from a page table. Does not free memory used
 * for the page table if the mapping was the last mapping present in the table.
 */
void ptable_unmap(struct pgd * pgd, void * virt_addr, int free) {
    struct pde * pde = GET_PDE(pgd, virt_addr);
    if(!PDE_EXISTS(pde)) {
        klog("ptable_unmap(pgd: 0x%x, va: 0x%x): PDE does not exist for VA\n");
        return;
    }

    /* Locate the Page Table Entry for this specific mapping */
    struct pgt * pgt = PDE_TO_PGT(pde);
    struct pte * pte = GET_PTE(pgt, virt_addr);

    /* Get the physical address of the page the PTE maps to */
    void * pte_pa = PTE_PA(pte);

    /* If a page has been allocated at the mapped address, free it */
    if(free) {
        uint32_t pfn = PA_TO_PFN(pte_pa);
        struct page * page = page_from_pfn(pfn);
        page_free(page, 0);
    }

    /* Clear the mapping */
    pte->address = 0;
    pte->present = 0;

    /* Invalidate the TLB entry for this address */
    paging_inval_tlb_entry(virt_addr);
}

/* ------------------------------------------------------------------------- */

/**
 * ptable_unmap_many() - Remove multiple page mappings from a page table.
 * @pgd:       The top-level page table (PGD) to removing the mappings from.
 * @virt_addr: The virtual address to unmap from.
 * @count:     How many pages to unmap.
 * @free:      Whether to free the physical page frames used.
 *
 * Frees multiple contiguous page mappings to a table by calling ptable_unmap()
 * for each page address in the range.
 */
void ptable_unmap_many(struct pgd * pgd, void * virt_addr, int count, int free) {
    for(int i = 0; i < count; i++) {
        ptable_unmap(pgd, virt_addr + i * PAGE_SIZE, free);
    }
}

/* ------------------------------------------------------------------------- */

/**
 * ptable_copy_range() - Copy a range of mappings from one PGD to another.
 * @source_pgd: The PGD to copy the mappings from.
 * @dest_pgd:   The PGD to copy the mappings to.
 * @start_addr: The starting address of the virtual address range to copy.
 * @end_addr:   The end address of the virtual address range to copy.
 * @flags:      Reserved.
 *
 * TODO
 */
void ptable_copy_range(struct pgd * source_pgd, struct pgd * dest_pgd,
                        void * start_addr, void * end_addr, flags_t flags) {

    klog("ptable_copy_range(src: 0x%x, dst: 0x%x, sa: 0x%x, ea: 0x%x)\n",
         source_pgd, dest_pgd, start_addr, end_addr);

    int start_pde = PAGE_DIRECTORY_INDEX(start_addr);
    int end_pde   = PAGE_DIRECTORY_INDEX(end_addr);
    int start_pte = PAGE_TABLE_INDEX(start_addr);
    int end_pte   = PAGE_TABLE_INDEX(end_addr);

    /* For each Page Directory Entry */
    for(int curr_pde = start_pde; curr_pde <= end_pde; curr_pde++) {
        /* Check that a PDE exists at this index in the source */
        struct pde * pde_old = &source_pgd->entries[curr_pde];
        if(!PDE_EXISTS(pde_old)) {
            continue;
        }

        /* Check if a PDE already exists in the destination table */
        struct pde * pde_new = &dest_pgd->entries[curr_pde];
        if(!PDE_EXISTS(pde_new)) {
            struct page * pde_page = page_alloc(0, PR_KERNEL);

            /* Set the new PDE entry to point to the new page */
            *pde_new = MAKE_PDE(PAGE_PA(pde_page),
                                PDE_PRESENT | PDE_WRITABLE | PDE_USER);
        }

        /* Get the PGTs for the PDE */
        struct pgt * pgt_new = PDE_TO_PGT(pde_new);
        struct pgt * pgt_old = PDE_TO_PGT(pde_old);

        /* Iterate through each page table entry - for the first PDE, we will
         * want to make sure that we start from the right index in the first
         * table and end at the right index in the last table. Any tables in
         * the middle, we'll cover entirely. */
        int curr_start_pte = 0;
        int curr_end_pte   = PAGE_TABLE_SIZE;
        if(curr_pde == start_pde) {
            curr_start_pte = start_pte; /* First PDE */
        } else if (curr_pde == end_pde) {
            curr_end_pte = end_pte;     /* End PDE */
        }

        for(; curr_start_pte < curr_end_pte; curr_start_pte++) {
            if(!PTE_EXISTS(&pgt_old->entries[curr_start_pte])) {
                continue;
            }

            struct pte * pte_new = &pgt_new->entries[curr_start_pte];
            struct pte * pte_old = &pgt_old->entries[curr_start_pte];

            if(TEST_BIT(flags, PTC_SHARE)) {
                /* New PTEs will refer to the same physical pages as the source
                 * table */
                void * pte_old_pa = PTE_PA(pte_old);
                struct page * page_old = PA_PAGE(pte_old_pa);
                PAGE_INC_USES(page_old);
                *pte_new = *pte_old;
            } else if(TEST_BIT(flags, PTC_COPY)) {
                /* New PTEs will refer to new physical pages containing the
                 * copied content of the original pages */
                void * copied_page = PAGE_VA(page_alloc(0, PR_KERNEL));
                void * orig_page   = PTE_VA(pte_old);

                /* Copy the contents of the source page to our new page */
                memcpy(copied_page, orig_page, PAGE_SIZE);

                /* Create a new PTE pointing to our new page */
                *pte_new = MAKE_PTE(VIR_TO_PHY(copied_page),
                                    PTE_PRESENT | PTE_USER);
            } else if(TEST_BIT(flags, PTC_COW)) {
                /* New PTEs will refer to the same physical pages as the source
                 * table, but will be copied to new pages upon a write */
                /* TODO */
            }
        }
    }
}

/* ------------------------------------------------------------------------- */

/**
 * ptable_pgt_is_clear() - Returns whether a page table is empty.
 * @pgt: The page table to inspect.
 *
 * Iterates through PAGE_TABLE_SIZE number of pte entries within the provided
 * pgt and validates that both their address and present flag are set to 0.
 *
 * Return: 0 if the table is not clear, 1 if it is clear.
 */
int ptable_pgt_is_clear(struct pgt * pgt) {
    for(int i = 0; i < PAGE_TABLE_SIZE; i++) {
        if(pgt->entries[i].address != 0 || pgt->entries[i].present != 0) {
            return 0;
        }
    }
    return 1;
}

/* ------------------------------------------------------------------------- */

/**
 * ptable_get_pte() - Get the Page Table Entry for an address in a PGD.
 * @pgd:       The top-level page directory to search.
 * @virt_addr: The virtual address to retrieve the PTE for.
 *
 * Retrieves a pointer to the page table entry for the given virtual address
 * in the target page global directory. Fails if no PDE can be found for
 * the virtual address. Can be used to retrieve the physical address for a
 * virtual mapping and its associated properties.
 *
 * Return: A pointer to the pte object for the address mapping.
 */
struct pte * ptable_get_pte(struct pgd * pgd, void * virt_addr) {
    struct pde * pde = GET_PDE(pgd, virt_addr);
    if(!PDE_EXISTS(pde)) {
        klog("ptable_get_pte(): No PDE found for va. 0x%x in PGD 0x%x\n",
             virt_addr, pgd);
        return NULL;
    }

    struct pgt * pgt = PDE_TO_PGT(pde);
    struct pte * pte = GET_PTE(pgt, virt_addr);

    return pte;
}

/* ------------------------------------------------------------------------- */
