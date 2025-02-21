#include <arch/multiboot.h>

extern uint32_t KERNEL_PHYS_END;

void udivmod64(uint64_t dividend, uint64_t divisor, uint64_t *quotient, uint64_t *remainder) {
    uint64_t q = 0;
    uint64_t r = 0;
    int i;

    for (i = 63; i >= 0; i--) {
        r <<= 1;
        r |= (dividend >> i) & 1;

        if (r >= divisor) {
            r -= divisor;
            q |= (uint64_t)1 << i;
        }
    }

    if (quotient) {
        *quotient = q;
    }

    if (remainder) {
        *remainder = r;
    }
}

void uint64_to_str(uint64_t value, char *str, int base) {
    char *ptr = str, *ptr1 = str, tmp_char;
    uint64_t quotient, remainder;

    // Calculate the string representation of the number in the given base
    do {
        udivmod64(value, base, &quotient, &remainder);
        value = quotient;
        *ptr++ = "0123456789abcdef"[remainder];
    } while (value);

    // Null-terminate the string
    *ptr-- = '\0';

    // Reverse the string
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}


uint32_t multiboot_parse(uint32_t mboot_magic, multiboot_info_t * info) {

    if(mboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        klog("Invalid Multiboot magic bytes! Got 0x%x, expected 0x%x\n", mboot_magic, 
                MULTIBOOT_BOOTLOADER_MAGIC);
        return 0;
    }

    klog("--- Multiboot Info ---\n");
    klog("Struct@:  0x%x\n", &info);
    klog("Flags:    0x%x\n", info->flags);
    klog("MemLow:   0x%x\n", info->mem_lower);
    klog("MemUpp:   0x%x\n", info->mem_upper);
    klog("MmapLen:  %d\n", info->mmap_length);
    klog("MmapAddr: 0x%x\n", info->mmap_addr);

    if (info->flags & MULTIBOOT_INFO_MEMORY) {
        klog("Lower memory: %dKB\n", info->mem_lower);
        klog("Upper memory: %dKB\n", info->mem_upper);
    }

    // Print boot device information
    if (info->flags & MULTIBOOT_INFO_BOOTDEV) {
        klog("Boot device: 0x%x\n", info->boot_device);
    }

    // Print command line information
    if (info->flags & MULTIBOOT_INFO_CMDLINE) {
        klog("Command line (at 0x%x): %s\n",
                         (char *)info->cmdline, (char *)info->cmdline);
    }

    // Print modules information
    if (info->flags & MULTIBOOT_INFO_MODS) {
        klog("Modules count: %d\n", info->mods_count);

        multiboot_module_t *mod;
        for (uint32_t i = 0; i < info->mods_count; i++) {
            mod = (multiboot_module_t *)(info->mods_addr + i * sizeof(multiboot_module_t));
            klog("Module %d start: 0x%x\n", i, mod->mod_start);
            klog("Module %d end: 0x%x\n", i, mod->mod_end);
            klog("Module %d command line: %s\n", i, (char *)mod->cmdline);
        }
    }

    // Print drive information
    /*
    if(info->flags & MULTIBOOT_INFO_DRIVE_INFO) {
        multiboot_drive_info_t * drive_info;
        klog("Drive count: %d\n", info->drives_length);
        for(uint32_t i = 0; i < info->drives_length; i++) {
            drive_info = (multiboot_drive_info_t*)(info->drives_addr + i);
            printk("Drive num: %d\n", (uint32)drive_info->drive_number);
        }
    }
    */

    // Print memory map information
    if (info->flags & MULTIBOOT_INFO_MEM_MAP) {
        multiboot_memory_map_t *mmap;

        for (mmap = (multiboot_memory_map_t *)info->mmap_addr;
             (unsigned long)mmap < info->mmap_addr + info->mmap_length;
             mmap = (multiboot_memory_map_t *)((unsigned long)mmap + mmap->size + sizeof(mmap->size))) {

            const char* type = (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) ? "Available" : "Reserved";

            char start_addr[32];
            char end_addr[32];
            uint64_to_str(mmap->addr, start_addr, 16);
            uint64_to_str(mmap->addr + mmap->len, end_addr, 16);


            klog("Memory region: 0x%s -> 0x%s (%s)\n",
                   start_addr,
                   end_addr,
                   type);

            if(mmap->addr <= (uint32_t)&KERNEL_PHYS_END && (mmap->addr + mmap->len) >= (uint32_t)&KERNEL_PHYS_END && mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {

                /*
                uint32_t aligned_end_addr = PAGE_ALIGN((uint32)&KERNEL_PHYS_END);
                uint32_t final_addr = mmap->addr + mmap->len;
                */

                klog("Found appropriate mem. region, start: 0x%x, end: 0x%x\n", mmap->addr, (mmap->addr + mmap->len));

                uintptr_t start_addr  = (uintptr_t)mmap->addr;
                uintptr_t end_addr    = (uintptr_t)(mmap->addr + mmap->len);
                uint32_t  region_type = (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) ? MEM_REGION_AVAILABLE : MEM_REGION_RESERVED;
                bootmem_add_mem_region(start_addr, end_addr, region_type);
            }
        }
    }

    // Print boot loader name
    if (info->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME) {
        klog("Boot loader name: %s\n", (char *)info->boot_loader_name);
    }

    return 1;
}
