/*
 * include/rotary/mm/palloc.h
 * Buddy Page Allocator
 */

#ifndef INC_MM_PALLOC_H
#define INC_MM_PALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include <rotary/list.h>
#include <rotary/mm/ptable.h>
#include <rotary/mm/bootmem.h>
#include <arch/paging.h>

/* ------------------------------------------------------------------------- */

#define ORDER_USED    -1
#define ORDER_DEFAULT 0
#define ORDER_MIN     0
#define ORDER_MAX     6

#define PR_KERNEL 1

/* page->flags values */
#define PF_INVALID        0x01 // If set, page cannot be used.
#define PF_ZONE_LOWMEM    0x02 // Page is directly mapped in kernel virt. mem.
#define PF_ZONE_HIGHMEM   0x04
#define PF_KERNEL         0x08 // Page contains fixed kernel code or structs

/* Page presence */
#define PAGE_NOT_PRESENT 0
#define PAGE_PRESENT     1

/* ------------------------------------------------------------------------- */

#define PFN_TO_PA(pfn) ((pfn) * PAGE_SIZE)
#define PFN_TO_VA(pfn) (PHY_TO_VIR(PFN_TO_PA((pfn))))

#define PA_TO_PFN(pa)  ((uint32_t)(pa) >> 12)
#define VA_TO_PFN(va)  (PA_TO_PFN(VIR_TO_PHY((va))))

#define PAGE_PA(page) ((void*)PFN_TO_PA(page->pfn))
#define PAGE_VA(page) ((void*)PHY_TO_VIR(PAGE_PA(page)))

#define PA_PAGE(pa)   (page_from_pfn(PA_TO_PFN(PAGE_FRAME((pa)))))
#define VA_PAGE(va)   (PA_PAGE(VIR_TO_PHY((va))))

#define SIZE_ORDER(size) ({ \
    int order = 0; \
    int pages = (size) / PAGE_SIZE; \
    while(pages > 1) { \
        pages >>= 1; \
        order++; \
    } \
    order; \
})

#define PAGE_INC_USES(page) ((page)->use_count++)
#define PAGE_DEC_USES(page) ((page)->use_count--)

/* ------------------------------------------------------------------------- */

/* Represents a physical page */
struct page {
    uint32_t    pfn;
    uint32_t    use_count;
    uint32_t    flags;
    int32_t     order;
    list_node_t buddy_node;
};

/* Manages free pages for a given order */
struct block_list {
    list_node_t free_pages;
    uint32_t    free_count;
    uint32_t    used_count;
};

/* Manages a buddy allocator instance */
struct buddy_allocator {
    struct page * page_area;
    uint32_t page_count;
    struct block_list * blocks;
    uint32_t max_order;
    volatile atomic_flag lock;
};

/* ------------------------------------------------------------------------- */

struct page * page_alloc(uint32_t order, uint32_t flags);
struct page * page_from_pfn(uint32_t pfn);
struct page * page_get_last(struct buddy_allocator * allocator,
                            uint32_t order);
struct page * buddy_get(struct page * page, uint32_t order);

int32_t page_free(struct page * current_page, int order);
void    page_initial_free(struct page * page);

int32_t  page_is_critical(struct page * page);
void *   page_area_end();

void     page_print_debug(struct page * page);

int32_t  buddy_init(uint32_t highest_pfn);
int32_t  buddy_split_block(uint32_t order);
int32_t  buddy_merge_block(struct page * block_page, uint32_t order);
void     buddy_remove_block(struct page * block_page);
void     buddy_add_block(struct page * block_page, uint32_t order);
void     buddy_print_debug();

/* ------------------------------------------------------------------------- */

static inline void page_mark_invalid(struct page * page) {
    page->flags |= PF_INVALID;
}

static inline void * page_alloc_pa(int order, flags_t flags) {
    return PAGE_PA(page_alloc(order, flags));
}

static inline void * page_alloc_va(int order, flags_t flags) {
    return PAGE_VA(page_alloc(order, flags));
}

static inline int32_t page_free_pa(void * pa, int order) {
    int pfn = PA_TO_PFN(pa);
    struct page * page = page_from_pfn(pfn);
    return page_free(page, order);
}

static inline int32_t page_free_va(void * va, int order) {
    int pfn = VA_TO_PFN(va);
    struct page * page = page_from_pfn(pfn);
    return page_free(page, order);
}

/* ------------------------------------------------------------------------- */

#endif
