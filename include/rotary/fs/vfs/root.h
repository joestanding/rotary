/*
 * include/rotary/fs/vfs/root.h
 */

#ifndef INC_FS_VFS_ROOT_H
#define INC_FS_VFS_ROOT_H

#include <rotary/core.h>
#include <rotary/debug.h>
#include <rotary/list.h>
#include <rotary/panic.h>
#include <rotary/fs/vfs/super.h>
#include <rotary/fs/vfs/inode.h>
#include <rotary/fs/vfs/fs_type.h>

/* TODO: Remove when initial implementation done */
#include <rotary/fs/testfs/super.h>

/* ------------------------------------------------------------------------- */

extern struct super_block * root_sb;
extern struct inode * root_inode;

/* ------------------------------------------------------------------------- */

void mount_root(int device, char * fs_type_name);
void mount_root_testing(); /* TODO: remove when block implementation done */

/* ------------------------------------------------------------------------- */

#endif
