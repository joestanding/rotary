/*
 * arch/x86/kernel/init.c
 * x86 Initialisation
 */

#include <rotary/arch_init.h>
#include <rotary/mm/slab.h>
#include <rotary/core/shell.h>

/* ------------------------------------------------------------------------- */

/**
 * arch_init() - Perform x86-specific initialisation.
 * @arg1: First argument passed to kernel_main(), in this case we expect this
 *        to be the Multiboot magic number.
 * @arg2: Second argument passed to kernel_main(), in this case we expect this
 *        to be a pointer to the Multiboot information struct.
 *
 * Initialise all of the necessary kernel subsystems, calling arch-specific
 * subsystems such as the Multiboot processor where necessary.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t arch_init(uint32_t arg1, uint32_t arg2) {

    uint32_t mboot_magic = arg1;
    multiboot_info_t * mboot_info = (multiboot_info_t*)arg2;

    vga_clear_screen(VGA_COLOUR_BLACK);
    vga_set_cursor(0, 23);

    // Initialise serial as early as we can for the sake of debug output
    printk(LOG_INFO, "Initialising serial ports..            ");
    if(!SUCCESS(serial_init())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    serial_write_line(serial_get_debug_port(),
    "\n\n"
    " ######                                    \n"
    " #     #  ####  #####   ##   #####  #   #  \n"
    " #     # #    #   #    #  #  #    #  # #   \n"
    " ######  #    #   #   #    # #    #   #    \n"
    " #   #   #    #   #   ###### #####    #    \n"
    " #    #  #    #   #   #    # #   #    #    \n"
    " #     #  ####    #   #    # #    #   #    \n"
    "                                           \n"
    "\n");

    printk(LOG_INFO, "Parsing Multiboot structs..            ");
    if(!multiboot_parse(mboot_magic, (multiboot_info_t*)mboot_info)) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Retrieving CPUID..                     ");
    if(!SUCCESS(cpuid_init())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Initialising paging..                  ");
    if(!SUCCESS(paging_init())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Initialising buddy allocator..         ");
    if(!SUCCESS(buddy_init(bootmem_highest_pfn()))) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);


    printk(LOG_INFO, "Initialising bootmem..                 ");
    if(!SUCCESS(bootmem_mark_free())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Setting up CPU..                       ");
    if(!SUCCESS(cpu_init())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Assigning IDT gates..                  ");
    if(!SUCCESS(idt_init())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Loading IDT..                          ");
    if(!SUCCESS(idt_load())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Configuring legacy PIC..               ");
    if(!SUCCESS(pic_init(PIC_MASTER_OFFSET, PIC_SLAVE_OFFSET))) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Initialising timer..                   ");
    if(!SUCCESS(timer_init())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Initialising keyboard driver..         ");
    if(!SUCCESS(keyboard_init())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Enabling interrupts..                  ");
    enable_hardware_interrupts();
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Initialising task scheduler..          ");
    if(!SUCCESS(task_init())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Initialising default TTYs..            ");
    if(!SUCCESS(tty_init())) {
        printk(LOG_INFO, FAIL_STR);
        return E_ERROR;
    }
    printk(LOG_INFO, OK_STR);

    task_create("shell", TASK_KERNEL, &shell_init, TASK_PRIORITY_MIN,
                TASK_STATE_WAITING);

    return 0;
}

/* ------------------------------------------------------------------------- */
