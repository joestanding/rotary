/**
 * arch/x86/include/arch/ptable.h
 * x86 Page Tables
 */

#ifndef INC_ARCH_PTABLE_H
#define INC_ARCH_PTABLE_H

#include <rotary/core.h>
#include <rotary/logging.h>
#include <arch/paging.h>

/* ------------------------------------------------------------------------- */

#define PAGE_DIR_SIZE   1024
#define PAGE_TABLE_SIZE 1024

/* Bit field flags for page directory entries (top level) */
#define PDE_PRESENT       0x01
#define PDE_WRITABLE      0x02
#define PDE_USER          0x04
#define PDE_WRITETHROUGH  0x08
#define PDE_CACHE_DISABLE 0x10
#define PDE_ACCESSED      0x20
#define PDE_PAGE_SIZE_4M  0x80
#define PDE_GLOBAL        0x100

/* Bit field flags for page table entries */
#define PTE_PRESENT       0x01
#define PTE_WRITABLE      0x02
#define PTE_USER          0x04
#define PTE_WRITETHROUGH  0x08
#define PTE_CACHE_DISABLE 0x10
#define PTE_ACCESSED      0x20
#define PTE_DIRTY         0x40
#define PTE_PAT           0x80
#define PTE_GLOBAL        0x100

/* Return the page frame start address for a PA */
#define PAGE_FRAME(addr) (((uint32_t)addr) & 0xFFFFF000)

/* Macros to ensure general alignment */
#define ALIGN(addr, alignment) ((uintptr_t)(addr) + ((alignment) - 1)) & \
                               ~((uintptr_t)(alignment) - 1)
#define ALIGN_DOWN(addr, alignment) ((uintptr_t)(addr) & \
                                    ~((uintptr_t)(alignment) - 1))

/* Macros to ensure page alignment */
#define PAGE_ALIGN(addr) ALIGN((addr), PAGE_SIZE)
#define PAGE_ALIGN_DOWN(addr) ALIGN_DOWN((addr), PAGE_SIZE)
#define IS_PAGE_ALIGNED(addr) (((uintptr_t)(addr) & (PAGE_SIZE - 1)) == 0)

/* Macros to derive page table indexes for an address */
#define PAGE_DIRECTORY_INDEX(addr) (((uint32_t)(addr) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(addr) (((uint32_t)(addr) >> 12) & 0x3FF)

/* Macros to derive an address from a directory or table index */
#define PDE_IDX_TO_ADDR(idx) ((idx) << 22)
#define PTE_IDX_TO_ADDR(idx) ((idx) << 12)

/* Macros to create page table entries with address and flags */
#define MAKE_PDE(addr, flags) ((struct pde){ \
    .entry = ((uintptr_t)(addr) & 0xFFFFF000) | ((flags) & 0xFFF) \
})
#define MAKE_PTE(addr, flags) ((struct pte){ \
    .entry = ((uintptr_t)(addr) & 0xFFFFF000) | ((flags) & 0xFFF) \
})

/* Macros to check for existence of PDEs and PTEs */
#define PDE_EXISTS(pde) (((pde)->entry & PDE_PRESENT) != 0)
#define PTE_EXISTS(pte) (((pte)->entry & PTE_PRESENT) != 0)

/* Get the relevant PDE or PTE for a virtual address */
#define GET_PDE(pgd, va) (&(pgd)->entries[PAGE_DIRECTORY_INDEX(va)])
#define GET_PTE(pgt, va) (&(pgt)->entries[PAGE_TABLE_INDEX(va)])

/* Get a pointer to the actual page table from a directory entry */
#define PDE_TO_PGT(pde) ((struct pgt*)PHY_TO_VIR(PAGE_FRAME((pde)->entry)))

#define PDE_PA(pde) ((void*)PAGE_FRAME((pde)->entry))
#define PTE_PA(pte) ((void*)PAGE_FRAME((pte)->entry))

#define PDE_VA(pde) (PHY_TO_VIR(PAGE_FRAME((pde)->entry)))
#define PTE_VA(pte) (PHY_TO_VIR(PAGE_FRAME((pte)->entry)))

#define PDE_IS_WRITABLE(pde) ((pde)->writable)
#define PDE_IS_USER(pde) ((pde)->user)

#define PDE_SET_WRITABLE(pde) ((pde)->writable = 1)
#define PDE_UNSET_WRITABLE(pde) ((pde)->writable = 0)

#define PDE_SET_USER(pde) ((pde)->user = 1)
#define PDE_UNSET_USER(pde) ((pde)->user = 0)

#define PDE_IS_HUGE(pde) ((pde)->page_size == 1)

#define PTE_IS_WRITABLE(pte) ((pte)->writable)
#define PTE_IS_USER(pte) ((pte)->user)

#define PTE_SET_WRITABLE(pte) ((pte)->writable = 1)
#define PTE_UNSET_WRITABLE(pte) ((pte)->writable = 0)

#define PTE_SET_USER(pte) ((pte)->user = 1)
#define PTE_UNSET_USER(pte) ((pte)->user = 0)

/* ------------------------------------------------------------------------- */
/* Page Directories and Tables                                               */
/*                                                                           */
/* On 32-bit x86, bits 31-22 of a virtual address provide the index into     */
/* the top-level page table.                                                 */
/*                                                                           */
/* Bits 21-12 provide the index into the second-level page table.            */
/* ------------------------------------------------------------------------- */

struct pde {
    union {
        uint32_t entry;
        struct {
            uint32_t present       : 1;
            uint32_t writable      : 1;
            uint32_t user          : 1;
            uint32_t writethrough  : 1;
            uint32_t cache_disable : 1;
            uint32_t accessed      : 1;
            uint32_t ignored       : 1;
            uint32_t page_size     : 1;
            uint32_t ignored2      : 4;
            uint32_t address       : 20;
        };
    };
};

struct pte {
    union {
        uint32_t entry;
        struct {
            uint32_t present       : 1;
            uint32_t writable      : 1;
            uint32_t user          : 1;
            uint32_t writethrough  : 1;
            uint32_t cache_disable : 1;
            uint32_t accessed      : 1;
            uint32_t dirty         : 1;
            uint32_t pat           : 1;
            uint32_t global        : 1;
            uint32_t ignored       : 3;
            uint32_t address       : 20;
        };
    };
};

struct pgt {
    struct pte entries[PAGE_TABLE_SIZE];
};

struct pgd {
    struct pde entries[PAGE_DIR_SIZE];
};

/* ------------------------------------------------------------------------- */

void ptable_print_pte(struct pte * pte);
void ptable_print_pde(struct pde * pde);
void ptable_print_pgd(struct pgd * pgd);
void ptable_print_pgt(struct pgt * pgt);

/* ------------------------------------------------------------------------- */

#endif
