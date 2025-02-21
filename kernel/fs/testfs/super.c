/*
 * kernel/fs/testfs/super.c
 */

#include <rotary/fs/testfs/super.h>

/* ------------------------------------------------------------------------- */

struct file_system_type testfs_type = {
    .name = "testfs",
};

/* ------------------------------------------------------------------------- */

void testfs_init() {
    file_system_type_register(&testfs_type);
}

/* ------------------------------------------------------------------------- */
