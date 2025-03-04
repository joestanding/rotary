/*
 * include/rotary/fs/vfs/inode.h
 * Virtual File System (VFS) Index Node
 *
 * The index node, or inode, represents a single file or object on a
 * filesystem. The inode contains all metadata and information about the file,
 * including its type, size, permissions, and location of the actual data on
 * a block device.
 *
 * The inode notably does not include a file/directory path or name - this is
 * handled separately.
 */

#ifndef INC_FS_VFS_INODE_H
#define INC_FS_VFS_INODE_H

#include <rotary/core.h>
#include <rotary/list.h>

/* ------------------------------------------------------------------------- */

struct inode_ops;

/* ------------------------------------------------------------------------- */

#define INIT_INODE(inode, mode, ops, _sb) \
    {                                     \
        (_inode)->mode = (_mode);            \
        (_inode)->ops = (_ops);              \
        (_inode)->sb = (_sb);                \
    }

/* ------------------------------------------------------------------------- */

struct inode {
    uint32_t mode; /* File type - regular, directory, symlink, etc. */
    struct inode_ops * ops;
    struct super_block * sb;
};

struct inode_ops {
    int (*lookup) (struct inode * dir_node, const char * name, size_t len, struct inode * result);
};

/* ------------------------------------------------------------------------- */

#endif
