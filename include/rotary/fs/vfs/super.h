/*
 * include/rotary/fs/vfs/super.h
 * Virtual File System (VFS) Super Block
 *
 * The super block represents a single mounted filesystem. It contains
 * information such as the filesystem type, the root inode, and a set of 
 * function pointers that allow operations to be executed on the mounted
 * filesystem.
 */

#ifndef INC_FS_VFS_SUPER_H
#define INC_FS_VFS_SUPER_H

#include <rotary/core.h>
#include <rotary/list.h>
#include <rotary/mm/kmalloc.h>
#include <rotary/fs/vfs/fs_type.h>
#include <rotary/fs/vfs/inode.h>
#include <rotary/fs/vfs/super.h>

/* ------------------------------------------------------------------------- */

struct inode;
struct file_system_type;
struct super_block_ops;

/* ------------------------------------------------------------------------- */

struct super_block {
    flags_t flags;
    struct inode root_inode;
    struct file_system_type * fs_type;
    list_head_t sb_list_node;
    struct super_block_ops * ops;
};

/* Super blocks have a configurable set of operations, defined by this struct
 * of function pointers. We have a set of high level operations we'll likely
 * want to conduct on filesystems of all types, and each filesystem can define
 * its own actual operations for each using this struct. */
struct super_block_ops {

};

/* ------------------------------------------------------------------------- */

struct super_block * super_block_alloc(struct file_system_type * fs);
void super_block_init(struct super_block * sb);
void super_block_register(struct super_block * sb);

/* ------------------------------------------------------------------------- */

#endif
