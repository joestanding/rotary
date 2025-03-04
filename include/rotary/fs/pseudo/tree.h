/*
 * include/rotary/fs/pseudo/tree.h
 */

#ifndef INC_FS_PSEUDO_TREE_H
#define INC_FS_PSEUDO_TREE_H

#include <rotary/core.h>
#include <rotary/list.h>

/* ------------------------------------------------------------------------- */

#define PSEUDO_FILE 0x01
#define PSEUDO_DIR  0x02

/* ------------------------------------------------------------------------- */

struct pseudo_node {
    char * name;
    flags_t mode;
    list_head_t children;
};

/* ------------------------------------------------------------------------- */

/*
static inline void pseudo_register_child(struct pseudo_node parent,
                                         struct pseudo_node child) {
    llist_
}
*/

/* ------------------------------------------------------------------------- */

#endif
