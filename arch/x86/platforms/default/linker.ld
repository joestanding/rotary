ENTRY("_start")

/* Rotary is loaded into physical memory at address 0x100000,
 * and mapped to 0xC0000000 once protected mode and paging are enabled. */
KERNEL_PMA  = 0x00100000;
KERNEL_VMA  = 0xC0000000;

SECTIONS
{

    /* Discard undesired ELF sections generated during compilation/assembly */
    /DISCARD/ : {
        *(.note.gnu.property)
        *(.note.GNU-stack)
        *(.comment)
        *(.iplt)
    }

    /* Set the location counter to the start physical addr
     * Addresses assigned and referenced within the binary will begin
     * at this address until we get to code that will execute after
     * paging has been enabled. */
    . = KERNEL_PMA;
    _kernel_start = .;

    /* Create a symbol for the kernel physical starting address for reference
     * within the operating system if desired. */
    PROVIDE(KERNEL_PHYS_START = _kernel_start);

    /* The Multiboot header must come first in the image */
    .multiboot :
    {
        build/output/arch/x86/boot/boot.o(.multiboot)
    }

    /* Our initial boot assembly code, which will bootstrap us then launch
     * the C kernel */
    .kernel_entry_text ALIGN(4K) :
    {
        build/output/arch/x86/boot/boot.o(.text)
        build/output/arch/x86/boot/boot.o(.bss)
    }
    _kernel_entry_end = .;

    /* All code from this point will only be executed once paging has been
     * enabled, and no mappings to the lower 0x1000000 of phys. memory will
     * exist anymore. Increment the location counter so all references
     * will now be within paged memory. */
    . += KERNEL_VMA;

    /* Our main kernel code. Use the AT() directive to ensure that the section
     * is loaded correctly at its intended physical address by subtracting
     * the virtual offset. This ensures the kernel code is loaded (and in the
     * image) in the 0x100000 region, but all of the references will be linked
     * within the 0xC0000000 paged region. */
    .kernel_text ALIGN (4K) : AT(ADDR(.kernel_text) - KERNEL_VMA)
    {
        build/output/kernel/*.o(.text)
        build/output/arch/x86/kernel/isr.o(.text)
        arch/x86/kernel/*.o(.text)
    }

    /* Define symbols so that we can access ktest modules stored within
     * this section easily from C */
    .ktest ALIGN (4K) :
    {
        __start_ktest = .;
        KEEP(*(.ktest))
        __stop_ktest = .;
    }
    
    .kernel_data ALIGN (4K) :
    {
        arch/x86/kernel/*.o(.data)
    	build/output/kernel/*.o(.data)
    	build/output/arch/x86/kernel/*.o(.data)
    	build/output/arch/x86/boot/*.o(.data)
    }

    .kernel_rodata ALIGN (4K) :
    {
        arch/x86/kernel/*.o(.rodata)
	    build/output/kernel/*.o(.rodata)
        build/output/arch/x86/kernel/*.o(.rodata)
    	build/output/arch/x86/boot/*.o(.rodata)
    }

    .kernel_bss ALIGN (4K) :
    {
    	build/output/kernel/*.o(.bss)
        arch/x86/kernel/*.o(.bss)
        build/output/arch/x86/kernel/*.o(.bss)
    	build/output/arch/x86/boot/*.o(.bss)
    }

    /* Create some more symbols for OS use if desired */
    _kernel_end = . - KERNEL_VMA;
    _kernel_virt_end = .;
    PROVIDE(KERNEL_PHYS_END = _kernel_end);
    PROVIDE(KERNEL_VIRT_END = _kernel_virt_end);

    . = 0x400000;
    .usermode_test :
    {
        build/output/arch/x86/kernel/usermode-test.o(.usermode_test)
    }

}
