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
    klog("Mounting root in TEST MODE\n");

    /* Call testfs_init() which also registers the filesystem type */
    testfs_init();

    /* Get a handle to the testfs filesystem type, and call mount(), which
     * will return a super_block we can register as root_sb */
    struct file_system_type * fs = file_system_type_get("testfs");
    if(!fs) {
        PANIC("Could not retrieve FS type for root volume!\n");
        return;
    }
    klog("FS type for rootfs at: 0x%x\n", fs);

    /* Allocate a super block */
    struct super_block * sb = fs->super_alloc();
    if(!sb) {
        PANIC("Could not allocate super block for root FS!\n");
        return;
    }
    klog("SB for rootfs at: 0x%x\n", sb);
    root_sb = sb;

    klog("Root SB: 0x%x\n", root_sb);

    klog("\n");
    debug_break();
}

/* ------------------------------------------------------------------------- */
