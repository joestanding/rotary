ENTRY("_start")

KERNEL_PMA = 0x80000;

SECTIONS {

    /* Discard undesired ELF sections generated during compilation/assembly */
    /DISCARD/ : {
        *(.note.gnu.property)
        *(.note.GNU-stack)
        *(.comment)
        *(.iplt)
    }

    /*
     * Set our starting memory address from 0x80000, as this is where code is
     * loaded to by the bootloader when loading a 64-bit image
     */
    . = KERNEL_PMA;
    _kernel_start = .;

    /* Create a symbol for the kernel physical starting address for reference
     * within the operating system if desired. */
    PROVIDE(KERNEL_PHYS_START = _kernel_start);

    __start = .;

    __text_start = .;
    .text :
    {
        KEEP(*(.text.boot))
        *(.text)
    }
    . = ALIGN(4096);
    __text_end = .;

    __rodata_start = .;
    .rodata :
    {
        *(.rodata)
    }
    . = ALIGN(4096);
    __rodata_end = .;

    __data_start = .;
    .data :
    {
        *(.data)
    }
    . = ALIGN(4096);
    __data_end = .;

    __bss_start = .;
    .bss : {
        bss = .;
        *(.bss)
    }
    . = ALIGN(4096);
    __bss_end = .;

    _end = .;

    /* Create a symbol for the kernel physical ending address */
    PROVIDE(KERNEL_PHYS_END = _end);

}
