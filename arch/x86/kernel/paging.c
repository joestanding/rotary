/**
 * arch/x86/kernel/paging.c
 * x86 Paging
 */

#include <arch/paging.h>

/* ------------------------------------------------------------------------- */

/* Symbols pointing to the beginning and end of the kernel's image in
 * physical memory, as provided by the linker script */
extern uintptr_t KERNEL_PHYS_START;
extern uintptr_t KERNEL_PHYS_END;

/* Page global directory for kernel tasks */
struct pgd kernel_pgd __attribute__((aligned(PAGE_SIZE)));

/* ------------------------------------------------------------------------- */

/**
 * paging_init() - Initialise the paging system.
 *
 * Return: E_SUCCESS
 */
int32_t paging_init() {
    klog("Initialising paging..\n");
    paging_setup_kernel_pgd();
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * paging_setup_kernel_pgd() - Set-up initial kernel page table.
 *
 * Populates the kernel page global directory with the kernel address space
 * mappings, then updates the current system page table.
 */
void paging_setup_kernel_pgd() {
    klog("Setting up kernel page table..\n");

    int table_cur;
    int table_start = PAGE_DIRECTORY_INDEX(KERNEL_START_VIRT);
    int table_kmap  = PAGE_DIRECTORY_INDEX(KMAP_START_VIRT);
    int pde_global  = (x86_paging_pge_enabled() ? PDE_GLOBAL : 0);
    int pte_global  = (x86_paging_pge_enabled() ? PTE_GLOBAL : 0);

    if(x86_paging_pse_enabled()) {
        klog("PSE available and enabled, using 4MB pages for kernel PGD\n");
        /* PSE available, we can use 4MB pages and avoid making page tables */
        for(table_cur = table_start; table_cur < table_kmap; table_cur++) {
            uint32_t phys_addr = PDE_IDX_TO_ADDR(table_cur - table_start);
            kernel_pgd.entries[table_cur] = MAKE_PDE(phys_addr, PDE_PRESENT |
                                                        PDE_WRITABLE |
                                                        PDE_PAGE_SIZE_4M |
                                                        pde_global);
        }
    } else {
        /* PSE is NOT available, use 4KB pages and create page tables */
        klog("PSE not available or enabled, using 4KB pages for kernel PGD\n");
        for(table_cur = table_start; table_cur < table_kmap; table_cur++) {
            struct pte * pte = bootmem_alloc(PAGE_SIZE, PAGE_SIZE);
            if(!pte) {
                PANIC("Failed to allocate memory for initial page tables!\n");
                return;
            }

            for(int page_cur = 0; page_cur < PAGE_TABLE_SIZE; page_cur++) {
                uint32_t phys_addr = PDE_IDX_TO_ADDR(table_cur - table_start) +
                                     PTE_IDX_TO_ADDR(page_cur);
                pte[page_cur] = MAKE_PTE(phys_addr, PTE_PRESENT |
                                                    PTE_WRITABLE |
                                                    pte_global);
            }

            kernel_pgd.entries[table_cur] = MAKE_PDE((uint32_t)VIR_TO_PHY(pte),
                                             PDE_PRESENT | PDE_WRITABLE |
                                             pde_global);
        }
    }

    /* Pre-allocate memory for kmap() page tables */
    klog("Setting up kmap() page tables..\n");
    for(table_cur = table_kmap; table_cur < PAGE_DIR_SIZE; table_cur++) {
        struct pte * pte = bootmem_alloc(PAGE_SIZE, PAGE_SIZE);
        if(!pte) {
            PANIC("Failed to allocate memory for kmap() tables!\n");
            return;
        }

        memset(pte, 0, PAGE_SIZE);

        kernel_pgd.entries[table_cur] = MAKE_PDE((uint32_t)VIR_TO_PHY(pte),
                                         PDE_PRESENT | PDE_WRITABLE |
                                         pde_global);
    }

    klog("Switching to new kernel page directory at 0x%x\n",
         VIR_TO_PHY(&kernel_pgd));
    paging_switch_pgd(VIR_TO_PHY(&kernel_pgd));
}

/* ------------------------------------------------------------------------- */

/**
 * paging_switch_pgd() - Change the current Page Global Directory (PGD).
 * @pgd: Pointer to the PGD to be loaded by the CPU.
 *
 * Changes the current PGD in use by the CPU, used when loading the initial
 * kernel page directory, can be used to change to any other page table.
 * Not used during task switching, as the PGD is saved and restored in the
 * task_context_switch() assembly routine.
 *
 * Return: E_SUCCESS on success E_ERROR if the PGD is not aligned
 */
int32_t paging_switch_pgd(struct pgd * pgd) {
    if(!pgd || !IS_PAGE_ALIGNED(pgd)) {
        klog("paging_switch_pgd(): PGD %x is not page aligned!\n",
               pgd);
        return E_ERROR;
    }

    klog("paging_switch_pgd(): Switching to PGD at 0x%x\n", pgd);

    asm volatile("mov %0, %%cr3" :: "r"((uint32_t)pgd) : "memory");
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * paging_kernel_pgd() - Retrieve a pointer to the kernel page directory.
 *
 * Return: A pointer to the kernel page directory.
 */
struct pgd * paging_kernel_pgd() {
    return &kernel_pgd;
}

/* ------------------------------------------------------------------------- */

/**
 * paging_handle_page_fault() - Handle a page fault exception
 * @registers: Pointer to the CPU registers at the time of the exception.
 *
 * This function is called when a page fault occurs. It retrieves the address
 * that caused the fault using the CR2 register and determines if the fault
 * can be resolved by the virtual memory subsystem. If the fault can be handled,
 * the necessary page table entry is created or updated.
 */
void paging_handle_page_fault(struct isr_registers * registers) {
    /* Get the task currently executing at the time */
    struct task * curr = cpu_get_local()->current_task;

    /* Retrieve the address we faulted at */
    void * fault_addr;
    __asm__ volatile("mov %%cr2, %0" : "=r" (fault_addr));

    /* Invoke arch-independent fault handler that will check if the fault
     * address is mapped and add it to the page table */
    if(SUCCESS(vm_space_page_fault(curr->vm_space, fault_addr))) {
        klog("paging_handle_page_fault(): VM subsystem resolved page fault\n");
        return;
    }

    /* TODO: Handle failure scenarios where the fault is not expected */
    klog("paging_handle_page_fault(): VM subsystem could NOT resolve!\n");
    debug_break();
}

/* ------------------------------------------------------------------------- */
