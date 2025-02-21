/**
 * include/rotary/mm/ptable.h
 * Arch-independent Page Table Operations
 */

#ifndef INC_MM_PTABLE_H
#define INC_MM_PTABLE_H

#include <rotary/core.h>
#include <rotary/mm/palloc.h>
#include <rotary/mm/vm.h>
#include <arch/paging.h>
#include <arch/ptable.h>

/* ------------------------------------------------------------------------- */

#define PTC_SHARE 0x01
#define PTC_COPY  0x02
#define PTC_COW   0x04

/* ------------------------------------------------------------------------- */

struct pgd * ptable_pgd_new();
void    ptable_pgd_free(struct pgd * pgd);
void    ptable_map(struct pgd * pgd, void * virt_addr, void * phys_addr,
                   flags_t flags);
void    ptable_map_many(struct pgd * pgd, void * virt_addr, void * phys_addr,
                        int count, flags_t flags);
void    ptable_unmap(struct pgd * pgd, void * virt_addr, int free);
void    ptable_unmap_many(struct pgd * pgd, void * virt_addr, int count,
                          int free);
void    ptable_copy_range(struct pgd * source_pgd, struct pgd * dest_pgd,
                          void * start_addr, void * end_addr, flags_t flags);
int     ptable_pgt_is_clear(struct pgt * pgt);
struct pte * ptable_get_pte(struct pgd * pgd, void *virt_addr);

/* ------------------------------------------------------------------------- */

#endif
