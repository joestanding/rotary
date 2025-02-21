/*
 * arch/x86/include/arch/gdt.h
 * x86 Global Descriptor Table
 */

#ifndef INC_ARCH_GDT_H
#define INC_ARCH_GDT_H

#include <rotary/core.h>
#include <rotary/logging.h>
#include <arch/tss.h>

/* ------------------------------------------------------------------------- */

#define GDT_ENTRY_COUNT       7

/* GDT Entry Indexes */
#define GDT_ENTRY_NULL        0
#define GDT_ENTRY_KERNEL_CODE 1
#define GDT_ENTRY_KERNEL_DATA 2
#define GDT_ENTRY_USER_CODE   3
#define GDT_ENTRY_USER_DATA   4
#define GDT_ENTRY_TSS         5
#define GDT_ENTRY_CPU_INFO    6

/* Descriptor Types */
#define GDT_ACCESS_TYPE_SEGMENT      0x10
#define GDT_ACCESS_TYPE_SYS_SEGMENT  0x00

/* Segment Descriptor */
#define GDT_ACCESS_PRESENT           0x80
#define GDT_ACCESS_NOT_PRESENT       0x00
#define GDT_ACCESS_PRIV_KERNEL       0x00
#define GDT_ACCESS_PRIV_USER         0x60
#define GDT_ACCESS_EXECUTABLE        0x08
#define GDT_ACCESS_DATA              0x00
#define GDT_ACCESS_DIRECTION_UP      0x00
#define GDT_ACCESS_DIRECTION_DOWN    0x04
#define GDT_ACCESS_CONFORMING        0x04
#define GDT_ACCESS_NONCONFORMING     0x00
#define GDT_ACCESS_READABLE          0x02
#define GDT_ACCESS_NOT_READABLE      0x00
#define GDT_ACCESS_WRITABLE          0x02
#define GDT_ACCESS_NOT_WRITABLE      0x00

/* System Segment Descriptor */
#define GDT_ACCESS_16BIT_TSS_AVAIL   0x01
#define GDT_ACCESS_LDT               0x02
#define GDT_ACCESS_16BIT_TSS_BUSY    0x03
#define GDT_ACCESS_32BIT_TSS_AVAIL   0x09
#define GDT_ACCESS_32BIT_TSS_BUSY    0x0B
#define GDT_ACCESS_64BIT_TSS_AVAIL   0x09
#define GDT_ACCESS_64BIT_TSS_BUSY    0x0B

/* GDT Offsets */
#define GDT_KERNEL_CODE_OFFSET       0x08
#define GDT_KERNEL_DATA_OFFSET       0x10
#define GDT_USER_CODE_OFFSET         0x18
#define GDT_USER_DATA_OFFSET         0x20

/* Privilege Levels (Rings) */
#define RPL_KERNELMODE               0x00
#define RPL_USERMODE                 0x03

/* ------------------------------------------------------------------------- */

/*
 * The GDT descriptor points to a list of GDT entries in memory
 * The total size of the entries minus one is in `size`, and
 * the address of the list is kept in `address`.
 */
typedef struct {
    unsigned short size;
    unsigned int   address;
} __attribute__((packed)) gdt_descriptor_t;

typedef struct {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access_flags;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed)) gdt_entry_t;

/* ------------------------------------------------------------------------- */

int32_t gdt_init();
void gdt_encode_entry(gdt_entry_t * entry, uint32_t base, uint32_t limit,
                      uint8_t flags, uint8_t access);
void gdt_print_debug_entry(const char * name, gdt_entry_t * entry);

/* ------------------------------------------------------------------------- */

#endif
