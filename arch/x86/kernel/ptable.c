/*
 * arch/x86/kernel/ptable.c
 */

#include <arch/ptable.h>

/* ------------------------------------------------------------------------- */

void ptable_print_pte(struct pte * pte) {
    char flag_str[256];
    memset(&flag_str, 0, sizeof(flag_str));

    if(TEST_BIT(pte->entry, PTE_PRESENT))
        strcat(flag_str, "Present, ");

    if(TEST_BIT(pte->entry, PTE_WRITABLE)) {
        strcat(flag_str, "Read/Write, ");
    } else {
        strcat(flag_str, "Read, ");
    }
    if(TEST_BIT(pte->entry, PTE_USER)) {
        strcat(flag_str, "Kernel & User, ");
    } else {
        strcat(flag_str, "Kernel Only, ");
    }
    if(TEST_BIT(pte->entry, PTE_WRITETHROUGH))
        strcat(flag_str, "Writethrough, ");
    if(TEST_BIT(pte->entry, PTE_CACHE_DISABLE))
        strcat(flag_str, "Cache Disable, ");
    if(TEST_BIT(pte->entry, PTE_ACCESSED))
        strcat(flag_str, "Accessed, ");
    if(TEST_BIT(pte->entry, PTE_DIRTY))
        strcat(flag_str, "Dirty, ");
    if(TEST_BIT(pte->entry, PTE_PAT))
        strcat(flag_str, "PAT, ");
    if(TEST_BIT(pte->entry, PTE_GLOBAL))
        strcat(flag_str, "Global, ");

    flag_str[strlen(flag_str)] = 0;
    flag_str[strlen(flag_str) - 1] = 0;

    klog("PTE[VA: 0x%x, Raw: 0x%x, Flags: %s]\n", pte, pte->entry, flag_str);
}

/* ------------------------------------------------------------------------- */

void ptable_print_pde(struct pde * pde) {
    char flag_str[256];
    memset(&flag_str, 0, sizeof(flag_str));

    if(TEST_BIT(pde->entry, PDE_PRESENT))
        strcat(flag_str, "Present, ");

    if(TEST_BIT(pde->entry, PDE_WRITABLE)) {
        strcat(flag_str, "Read/Write, ");
    } else {
        strcat(flag_str, "Read, ");
    }
    if(TEST_BIT(pde->entry, PDE_USER)) {
        strcat(flag_str, "Kernel & User, ");
    } else {
        strcat(flag_str, "Kernel Only, ");
    }
    if(TEST_BIT(pde->entry, PDE_WRITETHROUGH))
        strcat(flag_str, "Writethrough, ");
    if(TEST_BIT(pde->entry, PDE_CACHE_DISABLE))
        strcat(flag_str, "Cache Disable, ");
    if(TEST_BIT(pde->entry, PDE_ACCESSED))
        strcat(flag_str, "Accessed, ");
    if(TEST_BIT(pde->entry, PDE_PAGE_SIZE_4M))
        strcat(flag_str, "4MB Page, ");
    if(TEST_BIT(pde->entry, PDE_GLOBAL))
        strcat(flag_str, "Global, ");

    klog("PDE[VA: 0x%x, Raw: 0x%x, Flags: %s]\n", pde, pde->entry, flag_str);
}

/* ------------------------------------------------------------------------- */

void ptable_print_pgd(struct pgd * pgd) {
    for(int pde_idx = 0; pde_idx < PAGE_DIR_SIZE; pde_idx++) {
        struct pde * pde = &pgd->entries[pde_idx];
        if(!PDE_EXISTS(pde))
            continue;

        if(TEST_BIT(pde->entry, PDE_PAGE_SIZE_4M))
            continue;

        ptable_print_pde(pde);

        struct pgt * pgt = PDE_TO_PGT(pde);
        for(int pte_idx = 0; pte_idx < PAGE_TABLE_SIZE; pte_idx++) {
            struct pte * pte = &pgt->entries[pte_idx];
            if(!PTE_EXISTS(pte))
                continue;

            ptable_print_pte(pte);
        }
    }
}

/* ------------------------------------------------------------------------- */

void ptable_print_pgt(struct pgt * pgt) {

}

/* ------------------------------------------------------------------------- */
