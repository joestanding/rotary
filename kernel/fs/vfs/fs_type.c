/*
 * kernel/fs/vfs/fs_type.c
 */

#include <rotary/fs/vfs/fs_type.h>

list_head_t fs_list_head = INIT_LIST_HEAD(fs_list_head);
atomic_flag fs_list_lock = ATOMIC_FLAG_INIT;

/* ------------------------------------------------------------------------- */

void file_system_type_register(struct file_system_type * fs_type) {
    klog("Registering FS type '%s'\n", fs_type->name);

    lock(&fs_list_lock);
    clist_add(&fs_list_head, &fs_type->fs_list_entry);
    unlock(&fs_list_lock);
}

/* ------------------------------------------------------------------------- */

void file_system_type_unregister(struct file_system_type * fs_type) {
    klog("Unregistering FS type '%s'\n", fs_type->name);

    lock(&fs_list_lock);
    struct file_system_type * curr_entry;
    clist_for_each(curr_entry, &fs_list_head, fs_list_entry) {
        if(strcmp(curr_entry->name, fs_type->name) == 0) {
            clist_delete_node(&curr_entry->fs_list_entry);
        }
    }
    unlock(&fs_list_lock);
}

/* ------------------------------------------------------------------------- */

struct file_system_type * file_system_type_get(const char * name) {
    lock(&fs_list_lock);
    struct file_system_type * curr_entry;
    clist_for_each(curr_entry, &fs_list_head, fs_list_entry) {
        if(strcmp(curr_entry->name, name) == 0) {
            unlock(&fs_list_lock);
            return curr_entry;
        }
    }
    unlock(&fs_list_lock);

    return NULL;
}

/* ------------------------------------------------------------------------- */

void file_system_type_print_debug() {
    klog("Registered filesystem types:\n");
    struct file_system_type * curr_entry;
    clist_for_each(curr_entry, &fs_list_head, fs_list_entry) {
        klog("  Type: %s (0x%x)\n", curr_entry->name, curr_entry);
    }
}

/* ------------------------------------------------------------------------- */
