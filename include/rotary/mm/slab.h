/*
 * include/rotary/mm/slab.h
 * Slab Allocator for Kernel Heap
 */

#ifndef INC_MM_SLAB_H
#define INC_MM_SLAB_H

#include <rotary/core.h>
#include <rotary/logging.h>
#include <rotary/list.h>
#include <rotary/mm/palloc.h>

/* ------------------------------------------------------------------------- */

#define SLAB_DEFAULT_ORDER 4

/* ------------------------------------------------------------------------- */

#define INIT_SLAB_CACHE(name_str, obj_size) \
    {                                       \
        .name = name_str,                   \
        .object_size = obj_size,            \
        .total_size = 0,                    \
        .alloc_count = 0,                   \
        .first_slab = NULL                  \
    }

/* ------------------------------------------------------------------------- */

/* Free/empty objects will contain a pointer to the next free object */
struct slab_object_empty {
    struct slab_object_empty * next;
};

/* Slab: A contiguous set of physical pages used to store objects in a cache */
typedef struct slab_header {
    struct      slab_header * next_slab;
    void *      start_addr;     /* Addr. of first object in the cache */
    void *      end_addr;       /* Last address in the frame */
    uint32_t    object_count;   /* Total object count, both free and used */
    uint32_t    object_size;    /* Size of the object type stored in the slab */
    uint32_t    free_count;     /* Free objects in the slab */
    uint32_t    page_order;     /* Order of the phys. pages allocated */
    list_node_t slab_node;      /* A node within a cache's list of slabs */
    struct      slab_object_empty * free_list;
} slab_header_t;

/* Slab Cache: Contains objects of a fixed size */
typedef struct {
    char            name[16];     /* Name for debugging purposes */
    uint32_t        object_size;  /* Size of the object type stored in the cache */
    uint32_t        max_objects;  /* The maximum number of objects it can store */
    uint32_t        total_size;   /* Total memory space of the cache */
    uint32_t        alloc_count;  /* Total phys. pages allocated for the cache */
    atomic_flag     lock;         /* Spinlock */
    slab_header_t * first_slab;   /* Pointer to the first slab */
} slab_cache_t;

/* ------------------------------------------------------------------------- */

void *  slab_malloc(slab_cache_t * slab_cache, uint32_t flags);
int32_t slab_free(slab_cache_t * slab_cache, void * object);

void *  slab_alloc_from_cache(slab_cache_t * slab_cache);
void *  slab_alloc_from_slab(slab_header_t * header);

void    slab_add_cache_frame(slab_cache_t * slab_cache, uint32_t flags);

int32_t slab_cache_has_addr(slab_cache_t * slab_cache, void * addr);

void    slab_cache_print_debug(slab_cache_t * slab_cache);

/* ------------------------------------------------------------------------- */

#endif
