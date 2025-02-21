/**
 * arch/x86/include/arch/paging.h
 * x86 Paging
 */

#ifndef INC_ARCH_PAGING_H
#define INC_ARCH_PAGING_H

/* ------------------------------------------------------------------------- */

#define PAGE_SHIFT 12
#define PAGE_SIZE  (1 << PAGE_SHIFT)

#define KERNEL_START_VIRT 0xC0000000
#define KMAP_START_VIRT   0xF0000000
#define LOWMEM_PLIMIT     0x40000000

/* ------------------------------------------------------------------------- */

#define VIR_TO_PHY(addr) ((void *)((uintptr_t)(addr) - KERNEL_START_VIRT))
#define PHY_TO_VIR(addr) ((void *)((uintptr_t)(addr) + KERNEL_START_VIRT))

/* ------------------------------------------------------------------------- */

#include <rotary/core.h>
#include <rotary/mm/bootmem.h>
#include <rotary/sched/task.h>
#include <arch/cpuid.h>
#include <arch/interrupts.h>
#include <arch/ptable.h>

/* ------------------------------------------------------------------------- */

static inline void paging_inval_tlb_entry(void * addr) {
    asm volatile("invlpg (%0)" : : "r" (addr) : "memory");
}

/* ------------------------------------------------------------------------- */

struct isr_registers;
struct pgd;

/* ------------------------------------------------------------------------- */

int32_t paging_init();
void    paging_setup_kernel_pgd();
int32_t paging_switch_pgd(struct pgd * pgd);
struct pgd * paging_kernel_pgd();
void    paging_handle_page_fault(struct isr_registers * registers);

/* ------------------------------------------------------------------------- */

#endif
