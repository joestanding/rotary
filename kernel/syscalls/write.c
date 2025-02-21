#include <rotary/syscall.h>

void syscall_write(int descriptor_id, void * src_buffer, size_t size) {
    struct task * current_task = task_get_current();

    klog("Syscall: write\n");
    klog("File Descriptor: %d\n", descriptor_id);
    klog("Data Buffer:     0x%x\n", src_buffer);
    klog("Byte Count:      %d\n", size);
    /*

    // TODO: Check file descriptor validity
    if(descriptor_id >= TASK_DESCRIPTOR_MAX) {
        klog("file descriptor ID was out of bounds!\n");
    }

    // Check this descriptor is active
    if(current_task->descriptors[descriptor_id].type == DESCRIPTOR_TYPE_NONE) {
        klog("Unassigned descriptor specified!\n");
    }

    // Check src_buffer pointer is not NULL
    if(src_buffer == NULL) {
        klog("src_buffer was NULL!\n");
    }

    // Validate byte count
    if(size == 0) {
        klog("byte count was 0!\n");
    }

    list_node_t * current = &current_task->descriptors[0].list_entry;
    while(current != NULL) {
        current = current->next;
    }
    */
}
