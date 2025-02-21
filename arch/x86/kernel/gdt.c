/*
 * arch/x86/kernel/gdt.c
 * x86 Global Descriptor Table (GDT)
 *
 * Functions to initialise and configure the GDT on x86 platforms. The GDT is
 * used to define and characterise memory regions (known as segments).
 */

#include <arch/gdt.h>

extern void gdt_flush();

/* ------------------------------------------------------------------------- */

/**
 * gdt_encode_entry() - Create a GDT descriptor from parameters
 * @entry:  Pointer to the destination GDT entry struct
 * @base:   Base address where the segment begins in memory
 * @limit:  Maximum addressable offset within the segment
 * @flags:  Flags for granularity and segment size
 * @access: Flags defining presence, accessibility, privilege levels, and more
 *
 * Populates a Global Descriptor Table (GDT) entry with the provided base,
 * limit, and attributes. The function configures the entry to define a memory
 * segment with the specified characteristics.
 */
void gdt_encode_entry(gdt_entry_t * entry, uint32_t base, uint32_t limit,
                      uint8_t flags, uint8_t access) {

    entry->base_low    = (base & 0xFFFF);
    /* We shift the bits we want (17-24 of base) to the right by 16 so that
     * they now occupy the least significant bits (0-7). That allows us to then
     * extract them with the bitwise AND. */
    entry->base_middle = (base >> 16) & 0xFF;
    entry->base_high   = (base >> 24) & 0xFF;

    if (flags & 0x80) {  // Granularity bit (G) is set
        limit >>= 12;    // Convert byte limit to 4KB blocks
    }
    entry->limit_low   = (limit & 0xFFFF);
    entry->granularity = (limit >> 16) & 0x0F;
    entry->granularity |= flags & 0xF0;

    entry->granularity |= flags & 0xF0;
    entry->access_flags = access;
}

/* ------------------------------------------------------------------------- */

/**
 * gdt_print_debug_entry() - Prints debug information about a GDT entry.
 * @name:  A user-friendly name to be printed alongside the information.
 * @entry: A pointer to the GDT entry to be printed.
 */
void gdt_print_debug_entry(const char * name, gdt_entry_t * entry) {
    klog("%s:\n", name);
    klog("    b_low: 0x%x b_mid: 0x%x b_high: 0x%x\n",
                      entry->base_low,
                      entry->base_middle,
                      entry->base_high);
    klog("    gran: 0x%x access: 0x%x\n",
                      entry->granularity,
                      entry->access_flags);
}

/* ------------------------------------------------------------------------- */
