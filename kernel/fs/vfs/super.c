/*
 * kernel/fs/vfs/super.c
 * Virtual File System (VFS) Super Block
 */

#include <rotary/fs/vfs/super.h>

/* ------------------------------------------------------------------------- */

list_head_t sb_list = INIT_LIST_HEAD(sb_list);

/* ------------------------------------------------------------------------- */

/**
 * super_block_alloc() - Generic super block allocation
 *
 * Allocates a generic super block (SB) from the kernel heap. Suitable for
 * filesystem types that don't need to do anything special with the SB.
 *
 * Return: A pointer to the newly allocated super_block, or NULL if allocation
 *         fails.
 */
struct super_block * super_block_alloc(struct file_system_type * fs) {
    struct super_block * sb = kmalloc(sizeof(struct super_block)); 

    if(!sb) {
        klog("Failed to allocate new super block with kmalloc()!\n");
        return NULL;
    }

    super_block_init(sb);
    sb->fs_type = fs;

    return sb;
}

/* ------------------------------------------------------------------------- */

/**
 * super_block_init() - Initialise a super block with default values
 */
void super_block_init(struct super_block * sb) {
    memset(sb, 0, sizeof(struct super_block));
}

/* ------------------------------------------------------------------------- */

/**
 * super_block_register() - Add a super block to the global list
 * @sb: The super block to add to the list
 */
void super_block_register(struct super_block * sb) {
    llist_add(&sb_list, &sb->sb_list_node);
}

/* ------------------------------------------------------------------------- */
