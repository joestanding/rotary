/*
 * kernel/fs/testfs/super.c
 */

#include <rotary/fs/testfs/super.h>

/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */

struct file_system_type testfs_type = {
    .name = "testfs",
    .flags = 0x00000000,
    .super_alloc = testfs_super_alloc,
    .super_dealloc = NULL,
    .fs_list_entry = NULL
};

/* ------------------------------------------------------------------------- */

void testfs_init() {
    klog("Registering 'testfs' FS type..\n");
    file_system_type_register(&testfs_type);
}

/* ------------------------------------------------------------------------- */

struct super_block * testfs_super_alloc() {
    struct super_block * sb = super_block_alloc(&testfs_type);

    return sb;
}

/* ------------------------------------------------------------------------- */
