/*
 * include/rotary/fs/vfs/fs_type.h
 * File System Data and Functions
 */

#ifndef INC_FS_VFS_TYPE_H
#define INC_FS_VFS_TYPE_H

#include <rotary/core.h>
#include <rotary/list.h>
#include <rotary/logging.h>

/* ------------------------------------------------------------------------- */

struct super_block;

/* ------------------------------------------------------------------------- */

#define FS_TYPE_NODEV 0x01 /* Filesystem has no physical block device */

/* ------------------------------------------------------------------------- */

struct file_system_type {
    const char * name;      /* e.g. ext2, procfs */
    flags_t flags;          /* e.g. NODEV for pseudo filesystems */
    struct super_block *(*super_alloc) (void);
    void (*super_dealloc) (struct super_block * sb);
    list_node_t fs_list_entry;
};

/* ------------------------------------------------------------------------- */

void file_system_type_register(struct file_system_type * fs_type);
void file_system_type_unregister(struct file_system_type * fs_type);
struct file_system_type * file_system_type_get(const char * name);
void file_system_type_print_debug();

/* ------------------------------------------------------------------------- */

#endif
