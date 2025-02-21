/*
 * kernel/mm/kmalloc.c
 * Kernel Memory Allocator
 *
 * Provides general purpose memory allocations for kernel components. The slab
 * allocator is used to fulfil these memory allocations. kmalloc() cannot issue
 * memory allocations larger than the largest slab cache defined below.
 */

#include <rotary/mm/kmalloc.h>

/* Create slab caches for common allocation sizes - allocation requests for
 * sizes not specifically represented in here will round up to the most
 * appropriate slab cache if possible */
slab_cache_t slab_caches[] = {
    INIT_SLAB_CACHE("kmalloc-8", 8),
    INIT_SLAB_CACHE("kmalloc-16", 16),
    INIT_SLAB_CACHE("kmalloc-32", 32),
    INIT_SLAB_CACHE("kmalloc-64", 64),
    INIT_SLAB_CACHE("kmalloc-128", 128),
    INIT_SLAB_CACHE("kmalloc-256", 256),
    INIT_SLAB_CACHE("kmalloc-512", 512),
    INIT_SLAB_CACHE("kmalloc-1k", 1024),
    INIT_SLAB_CACHE("kmalloc-2k", 2048),
    INIT_SLAB_CACHE("kmalloc-4k", 4096),
    INIT_SLAB_CACHE("kmalloc-8k", 8192),
    INIT_SLAB_CACHE("kmalloc-16k", 16384),
    INIT_SLAB_CACHE("kmalloc-32k", 32768)
};

/* ------------------------------------------------------------------------- */

/**
 * kmalloc() - Allocate general purpose memory of a given size.
 * @size: The amount of bytes of memory to be allocated.
 *
 * Used as the primary generic purpose memory allocator for kernel components.
 * Memory for kmalloc() allocations is provided by the slab allocator, and an
 * array of pre-defined slab caches are created for common allocation sizes.
 *
 * If there is no slab cache for objects of the specific size requested,
 * the first slab cache larger than the requested size will be used if
 * available (for example an allocation request for 60 bytes will be fulfilled
 * using the 64 byte slab cache).
 *
 * Kernel components that require page-aligned regions of memory or memory
 * with specific attributes are most likely best served by retrieving pages
 * with page_alloc() instead.
 *
 * Return: A pointer to the allocation if successful, otherwise NULL if not.
 */
void * kmalloc(uint32_t size) {
    klog("kmalloc(): %d bytes requested\n", size);

    for(uint32_t i = 0; i < ARRAY_SIZE(slab_caches); i++) {
        if(size <= slab_caches[i].object_size) {
            klog("slab cache %d is acceptable, obj. size %d\n", i,
                 slab_caches[i].object_size);
            return slab_malloc(&slab_caches[i], 0);
        }
    }

    klog("kmalloc(): no suitable slab caches were found\n");

    return NULL;
}

/* ------------------------------------------------------------------------- */

/**
 * kfree() - Free a memory allocation.
 * @addr: The address of the object to be freed.
 *
 * Iterates through each of the slab caches used by kmalloc, and checks whether
 * the object to be freed is within the slab cache. If so, calls slab_free().
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t kfree(void * addr) {
    for(uint32_t i = 0; i < 13; i++) {
        if(slab_cache_has_addr(&slab_caches[i], addr)) {
            slab_free(&slab_caches[i], addr);
            return E_SUCCESS;
        }
    }
    klog("kfree(): Failed to find slab cache for addr. 0x%x!\n", addr);
    return E_ERROR;
}

/* ------------------------------------------------------------------------- */

/**
 * kmalloc_print_debug() - Print debug information about kmalloc slab caches.
 */
void kmalloc_print_debug() {
    for(uint32_t i = 0; i < 13; i++) {
        slab_cache_print_debug(&slab_caches[i]);
    }
}

/* ------------------------------------------------------------------------- */
