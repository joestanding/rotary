/*
 * kernel/mm/slab.c
 * Slab Allocator
 *
 * Enables fast and efficient allocation of memory for objects of fixed sizes.
 * "Caches" manage memory for objects of a given size, using "slabs" of
 * pre-allocated memory to store them.
 *
 * For example, a cache may be created to efficiently allocate memory for
 * objects of a 64 byte size. The cache would maintain a list of slabs, which
 * are blocks of contiguous memory used to store the cache's objects. These
 * slabs are allocated via the buddy page allocator with page_alloc().
 *
 * Used as the backing allocator for kmalloc().
 */

#include <rotary/mm/slab.h>

/* ------------------------------------------------------------------------- */

/**
 * slab_malloc() - Allocate an object from a slab cache.
 * @slab_cache: A pointer to the slab cache to allocate from.
 * @flags:      Reserved for future use.
 *
 * This function allocates an object from the given slab cache. It iterates
 * through each slab in the cache to find one with available space. If a slab
 * with free objects is found, it allocates an object from that slab. If no
 * free slab is found, it attempts to add a new slab to the cache and retries
 * the allocation.
 *
 * Return: A pointer to the allocated object, or NULL if the allocation fails.
 */
void * slab_malloc(slab_cache_t * slab_cache, uint32_t flags) {
    /* Attempt to allocate from an existing cache */
    void * new_object = slab_alloc_from_cache(slab_cache);
    if(new_object != NULL) {
        return new_object;
    }

    /* Add a new cache frame if no space was available */
    slab_add_cache_frame(slab_cache, 0);

    new_object = slab_alloc_from_cache(slab_cache);
    if(new_object != NULL) {
        return new_object;
    }

    klog("slab_malloc(): failed to alloc. new slab and issue object!\n");
    return NULL;
}

/* ------------------------------------------------------------------------- */

/**
 * slab_free() - Free an object in a slab cache.
 * @slab_cache: Pointer to the slab cache containing the slabs.
 * @object:     Pointer to the object to be freed.
 *
 * This function iterates through each slab in the given slab cache to find
 * the slab containing the specified object. Once the appropriate slab is
 * found, the object is added back to the slab's free list, and the free
 * count is incremented.
 *
 * Return: E_SUCCESS if the object was successfully freed, or E_ERROR
 *         otherwise.
 */
int32_t slab_free(slab_cache_t * slab_cache, void * object) {
    klog("slab_free(): freeing obj. 0x%x\n", object);

    /* Iterate through each slab in the cache and identify which slab's
     * address range it falls within */
    slab_header_t ** curr_slab = &slab_cache->first_slab;

    while (*curr_slab != NULL) {
        uintptr_t start_addr = (uintptr_t)(*curr_slab)->start_addr;
        uintptr_t end_addr   = start_addr + (PAGE_SIZE << (*curr_slab)->page_order);

        if ((uintptr_t)object >= start_addr && (uintptr_t)object < end_addr) {
            struct slab_object_empty ** curr_obj = &(*curr_slab)->free_list;
            while (*curr_obj != NULL) {

                if ((uintptr_t)object < (uintptr_t)*curr_obj) {
                    struct slab_object_empty * new_free_obj = (struct slab_object_empty *)object;
                    new_free_obj->next = *curr_obj;
                    *curr_obj = new_free_obj;

                    (*curr_slab)->free_count++;

                    return E_SUCCESS;
                }

                curr_obj = &((*curr_obj)->next);
            }

            /* If curr_obj is NULL, we reached the end of the free list */
            struct slab_object_empty * new_free_obj =
                                       (struct slab_object_empty *)object;
            new_free_obj->next = NULL;
            *curr_obj = new_free_obj;
            (*curr_slab)->free_count++;

            return E_SUCCESS;
        }

        /* Search the next slab in the cache */
        curr_slab = &((*curr_slab)->next_slab);
    }

    klog("slab_free(): object 0x%x not found in any slab!\n",
         (uintptr_t)object);
    return E_ERROR;
}

/* ------------------------------------------------------------------------- */

/**
 * slab_alloc_from_cache() - Allocate an object from a slab cache.
 * @slab_cache: Pointer to the slab cache to allocate from.
 *
 * This function iterates through each slab in the cache to find one with
 * available space. If a slab with free objects is found, it allocates an
 * object from that slab by calling slab_alloc_from_slab().
 *
 * Return: A pointer to the allocated object, or NULL if no free slab is found.
 */
void * slab_alloc_from_cache(slab_cache_t * slab_cache) {
    /* Iterate through each slab in the cache to find one with free space */
    slab_header_t ** curr_slab = &slab_cache->first_slab;
    while(*curr_slab != NULL) {
        if((*curr_slab)->free_count > 0) {
            klog("slab_alloc_from_cache() found suitable slab at 0x%x\n",
                              *curr_slab);
            return slab_alloc_from_slab((*curr_slab));
        }
        curr_slab = &((*curr_slab)->next_slab);
    }

    return NULL;
}

/* ------------------------------------------------------------------------- */

/**
 * slab_alloc_from_slab() - Allocate an object from a specific slab.
 * @header: Pointer to the slab header from which to allocate an object.
 *
 * This function allocates an object from the specified slab by removing
 * it from the slab's free list. It also updates the free count of the slab.
 *
 * Return: A pointer to the allocated object, or NULL if the allocation fails
 *         (e.g., if the slab has no free objects).
 */
void * slab_alloc_from_slab(slab_header_t * header) {
    if(header->free_count == 0) {
        klog("alloc_from_slab() called on slab w/o free objs!\n");
        return NULL;
    }

    if(header->free_list == NULL) {
        klog("Slab free list was NULL!\n");
        return NULL;
    }

    /* Get a pointer to the object we're going to allocate, it will be the
     * first object in the slab's free list */
    struct slab_object_empty * new_obj = header->free_list;

    /* Update the slab's free list so that it now points to the next free
     * object after our newly allocated one */
    header->free_list = header->free_list->next;

    /* Decrease the free object count for the slab */
    header->free_count--;

    return new_obj;
}

/* ------------------------------------------------------------------------- */

/**
 * slab_add_cache_frame() - Adds backing page frames to a slab cache.
 * @slab_cache: The slab cache to add new pages to.
 * @flags:      Reserved for future use.
 *
 * This function allocates new pages for the slab cache and initializes
 * them as a new slab. It updates the slab cache metadata to include the
 * new slab and initializes the free list for the objects in the slab.
 */
void slab_add_cache_frame(slab_cache_t * slab_cache, uint32_t flags) {
    klog("slab_add_frame(): Adding to slab_cache at 0x%x\n",
                      slab_cache);

    /* Allocate page(s) for our slab cache and its header */
    struct page * new_page = page_alloc(SLAB_DEFAULT_ORDER, PR_KERNEL);

    /* Handle memory exhaustion */
    if(!new_page) {
        klog("slab_add_cache_frame(): Failed to alloc. mem!\n");
        return;
    }

    /* Place the cache header at the very start of the allocated page(s) */
    slab_header_t * header = (slab_header_t*)PAGE_VA(new_page);

    /* Calculate available memory and the maximum object count */
    header->page_order    = SLAB_DEFAULT_ORDER;
    uint32_t page_count   = 1UL << SLAB_DEFAULT_ORDER;
    uint32_t total_size   = page_count * PAGE_SIZE;
    uint32_t usable_size  = total_size - sizeof(slab_header_t);
    uint32_t object_count = usable_size / slab_cache->object_size;

    /* Assign information about the slab */
    void * page_vaddr    = (void*)PAGE_VA(new_page);
    header->start_addr   = page_vaddr + sizeof(slab_header_t);
    header->end_addr     = page_vaddr + total_size;
    header->object_count = object_count;
    header->object_size  = slab_cache->object_size;
    header->free_count   = object_count;
    header->next_slab    = NULL;

    /* Update the slab cache metadata */
    slab_cache->total_size  += total_size;
    slab_cache->alloc_count += page_count;

    /* Add the new slab to the slab cache's list */
    slab_header_t ** curr_slab = &slab_cache->first_slab;
    while(*curr_slab != NULL) {
        klog("curr_slab was not NULL, iterating\n");
        curr_slab = &((*curr_slab)->next_slab);
    }
    *curr_slab = PAGE_VA(new_page);

    /* Initialize the freelist: each empty object contains a pointer to the
     * next free object, using slab_object_empty struct */
    struct slab_object_empty *prev_object = NULL;
    struct slab_object_empty *current_object = (struct slab_object_empty *)header->start_addr;

    for(uint32_t i = 0; i < header->object_count; i++) {
        if (prev_object) {
            prev_object->next = current_object;
        } else {
            header->free_list = current_object;
        }

        prev_object = current_object;
        current_object = (struct slab_object_empty *)((char *)current_object + slab_cache->object_size);
    }

    if (prev_object) {
        prev_object->next = NULL;
    }

}

/* ------------------------------------------------------------------------- */

/**
 * slab_cache_has_addr() - Returns whether the allocator holds an address.
 * @slab_cache: The slab cache to search within.
 * @addr:       The address to search for.
 *
 * Iterates through each slab cache held by the allocator, and assesses whether
 * the provided address falls within its range.
 *
 * Return: 1 if the address is within the slab allocator, 0 if not.
 */
int32_t slab_cache_has_addr(slab_cache_t * slab_cache, void * addr) {
    slab_header_t * curr_slab = slab_cache->first_slab;
    while(curr_slab) {
        if(addr >= curr_slab->start_addr && addr < curr_slab->end_addr)
            return 1;
        curr_slab = curr_slab->next_slab;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

/**
 * slab_cache_print_debug() - Print cache information to the kernel log.
 * @slab_cache: Pointer to the slab cache to print debug information for.
 *
 * This function prints detailed information about the slab cache, including
 * its name, address, object size, total size, allocation count, and details
 * about each slab in the cache.
 */
void slab_cache_print_debug(slab_cache_t * slab_cache) {
    klog("Cache '%s' [addr: 0x%x, objsize: %d]\n",
         slab_cache->name, slab_cache, slab_cache->object_size);

    slab_header_t ** curr_slab = &slab_cache->first_slab;
    if(*curr_slab == NULL) {
        klog("  No slabs!\n");
    }
    while(*curr_slab != NULL) {
        uint32_t  page_count  = (1 << (*curr_slab)->page_order);
        uint32_t  total_bytes = page_count * PAGE_SIZE;
        uintptr_t start_addr  = (uintptr_t)*curr_slab;
        uintptr_t end_addr    = start_addr + total_bytes;
        klog("  -> Slab[addr: 0x%x -> 0x%x, pages: %d, totalbytes: %d, "
             "order: %d, objtotal: %d, objfree: %d, objused: %d]\n",
             start_addr, end_addr, page_count, total_bytes,
             (*curr_slab)->page_order, (*curr_slab)->object_count,
             (*curr_slab)->free_count,
             (*curr_slab)->object_count - (*curr_slab)->free_count);
        curr_slab = &((*curr_slab)->next_slab);
    }

    klog("\n");
}

/* ------------------------------------------------------------------------- */
