/*
 * arch/x86/kernel/cpu.c
 * CPU Initialisation
 */

#include <arch/cpu.h>

/* ------------------------------------------------------------------------- */

extern void gdt_flush();
struct cpu_info cpu0;

/* ------------------------------------------------------------------------- */

/**
 * cpu_init() - Set up and initialise the CPU and its data structures
 *
 * Initialise the CPU info struct and the Global Descriptor Table. Can be used
 * in future to initialise other CPU data structures and capabilities.
 *
 * Return: E_SUCCESS
 */
int32_t cpu_init() {
    klog("cpu_init() - Initialising CPU..\n");

    /* Initialise the CPU info struct */
    memset(&cpu0, 0, sizeof(struct cpu_info));
    cpu0.cpu_id = 0;
    cpu0.self = &cpu0;

    /* Initialise the Global Descriptor Table */
    cpu_init_gdt(&cpu0);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * cpu_init_gdt() - Initialise the Global Descriptor Table.
 * @cpu: The cpu_info struct for the relevant CPU.
 *
 * Creates GDT descriptors for kernel code, kernel data, user code, user data,
 * and the Task State segment. Each segment is configured to cover all of the
 * maximum 4GB memory range available on 32-bit x86, as paging is used later
 * on to provide more fine-grained control over memory regions.
 */
void cpu_init_gdt(struct cpu_info * cpu) {
    klog("cpu_init_gdt() - Initialising GDT..\n");

    uint16_t gdt_size = (sizeof(gdt_entry_t) * GDT_ENTRY_COUNT) - 1;
    cpu->gdt_desc.address = (uint32_t)&cpu->gdt_entries[GDT_ENTRY_NULL];
    cpu->gdt_desc.size = gdt_size;

    /* Kernel code */
    gdt_encode_entry(&cpu->gdt_entries[GDT_ENTRY_KERNEL_CODE],
                     0x00000000, 0xFFFFFFFF, 0xCF,
                     GDT_ACCESS_TYPE_SEGMENT |
                     GDT_ACCESS_PRESENT |
                     GDT_ACCESS_PRIV_KERNEL |
                     GDT_ACCESS_EXECUTABLE |
                     GDT_ACCESS_NONCONFORMING |
                     GDT_ACCESS_READABLE);

    /* Kernel data */
    gdt_encode_entry(&cpu->gdt_entries[GDT_ENTRY_KERNEL_DATA],
                     0x00000000, 0xFFFFFFFF, 0xCF,
                     GDT_ACCESS_TYPE_SEGMENT |
                     GDT_ACCESS_PRESENT |
                     GDT_ACCESS_PRIV_KERNEL |
                     GDT_ACCESS_DATA |
                     GDT_ACCESS_DIRECTION_UP |
                     GDT_ACCESS_WRITABLE);

    /* User code */
    gdt_encode_entry(&cpu->gdt_entries[GDT_ENTRY_USER_CODE],
                     0x00000000, 0xFFFFFFFF, 0xCF,
                     GDT_ACCESS_TYPE_SEGMENT |
                     GDT_ACCESS_PRESENT |
                     GDT_ACCESS_PRIV_USER |
                     GDT_ACCESS_EXECUTABLE |
                     GDT_ACCESS_NONCONFORMING |
                     GDT_ACCESS_READABLE);

    /* User data */
    gdt_encode_entry(&cpu->gdt_entries[GDT_ENTRY_USER_DATA],
                     0x00000000, 0xFFFFFFFF, 0xCF,
                     GDT_ACCESS_TYPE_SEGMENT |
                     GDT_ACCESS_PRESENT |
                     GDT_ACCESS_PRIV_USER |
                     GDT_ACCESS_DATA |
                     GDT_ACCESS_DIRECTION_UP |
                     GDT_ACCESS_WRITABLE);

    /* Task State Segment */
    cpu->tss.ss0 = GDT_KERNEL_DATA_OFFSET;
    gdt_encode_entry(&cpu->gdt_entries[GDT_ENTRY_TSS], (uint32_t)&cpu->tss,
                     sizeof(struct tss), 0x40, 0x89);

    /* CPU-local data (GS) */
    gdt_encode_entry(&cpu->gdt_entries[GDT_ENTRY_CPU_INFO],
                 (uint32_t)&cpu->self,
                 sizeof(struct cpu_info) - 1,
                 0x4F,
                 GDT_ACCESS_TYPE_SEGMENT |
                 GDT_ACCESS_PRESENT |
                 GDT_ACCESS_PRIV_KERNEL |
                 GDT_ACCESS_DATA |
                 GDT_ACCESS_DIRECTION_UP |
                 GDT_ACCESS_WRITABLE);

    /* Load the GDT descriptor */
    cpu_load_gdtr(&cpu->gdt_desc);

    /* Set GS to point to the CPU-local struct */
    cpu_set_gs(GDT_ENTRY_CPU_INFO << 3);

    /* Load the right segment selectors */
    gdt_flush();

    /* Load the task register */
    tss_flush();
}

/* ------------------------------------------------------------------------- */

/**
 * cpu_load_gdtr() - Load the Global Descriptor Table register
 * @gdtr: Pointer to the GDT descriptor containing the base address and size
 *
 * Loads the GDTR register with the given GDT descriptor, making the specified
 * Global Descriptor Table active for memory segmentation.
 */
void cpu_load_gdtr(gdt_descriptor_t * gdtr) {
    asm volatile("lgdt %0" :: "m"(*gdtr));
}

/* ------------------------------------------------------------------------- */

/**
 * cpu_set_gs() - Set the GS segment register
 * @value: The selector value to load into the GS register
 *
 * Updates the GS segment register to point to a specific segment in the
 * Global Descriptor Table. Used for CPU-local data.
 */
void cpu_set_gs(uint16_t value) {
    asm volatile("movw %0, %%gs" : : "r"(value));
}

/* ------------------------------------------------------------------------- */

/**
 * cpu_get_local() - Retrieve the pointer to the CPU-local data structure
 *
 * Accesses the GS segment base and retrieves the address of the CPU-local
 * data structure. This is used for accessing per-CPU data in multiprocessor
 * environments.
 *
 * Return: Pointer to the current CPU's local data structure.
 */
struct cpu_info * cpu_get_local() {
    struct cpu_info * cpu;
    asm("movl %%gs:0, %0" : "=r"(cpu));
    return cpu;
}

/* ------------------------------------------------------------------------- */
