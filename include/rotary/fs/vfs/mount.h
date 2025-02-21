/*
 * include/rotary/fs/vfs/mount.h
 * Virtual File System (VFS) Mounting
 */

#ifndef INC_FS_VFS_MOUNT_H
#define INC_FS_VFS_MOUNT_H

#include <rotary/core.h>
#include <rotary/list.h>
#include <rotary/fs/vfs/fs_type.h>
#include <rotary/fs/vfs/inode.h>
#include <rotary/fs/vfs/super.h>

/* ------------------------------------------------------------------------- */

struct mount {
    struct super_block * super;
    struct inode * root_inode;
    list_node_t mount_list_node;
};

/* ------------------------------------------------------------------------- */

int vfs_mount();

/* ------------------------------------------------------------------------- */

#endif
