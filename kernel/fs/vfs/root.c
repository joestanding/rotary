/*
 * kernel/fs/vfs/root.c
 */

#include <rotary/fs/vfs/root.h>

/* ------------------------------------------------------------------------- */

struct super_block * root_sb;
struct inode * root_inode;
list_head_t mount_list = INIT_LIST_HEAD(mount_list);

/* ------------------------------------------------------------------------- */

/**
 * mount_root() - Mount a device to the root of the Virtual File System.
 */
void mount_root(int device, char * fs_type_name) {
    
}

/* ------------------------------------------------------------------------- */

/**
 * mount_root_testing() - Temporary root setup for testing FS
 *
 * TO BE REMOVED WHEN BLOCK IMPLEMENTATION COMPLETE
 */
void mount_root_testing() {
    /* Call testfs_init() which also registers the filesystem type */

    /* Get a handle to the testfs filesystem type, and call mount(), which
     * will return a super_block we can register as root_sb */

}

/* ------------------------------------------------------------------------- */
