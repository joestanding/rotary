/*
 * include/rotary/fs/vfs/namei.h
 * Virtual File System (VFS) Path Resolution
 */

#ifndef INC_FS_VFS_NAMEI_H
#define INC_FS_VFS_NAMEI_H

#include <rotary/core.h>
#include <rotary/list.h>

/* ------------------------------------------------------------------------- */

struct walk_data {
    
};

/* ------------------------------------------------------------------------- */

int namei(struct walk_data * data);

/* ------------------------------------------------------------------------- */

#endif
