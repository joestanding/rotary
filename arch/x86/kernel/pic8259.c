/*
 * arch/x86/kernel/pic8259.c
 * x86 8259 Legacy Programmable Interrupt Controller
 *
 * Functions to initialise and configure the legacy PIC present on x86 systems.
 * Will be emulated on just about anything from this century.
 */

#include <arch/pic8259.h>

/* ------------------------------------------------------------------------- */

/**
 * pic_send_eoi() - Send an End-of-Interrupt (EOI) signal to the relevant PIC.
 * @irq: The interrupt number for which we're sending an EOI.
 *
 * Sends an EOI to the PIC relevant for the specified interrupt number - this
 * is identified by the range of the interrupt number.
 *
 * Return: E_SUCCESS
 */
int32_t pic_send_eoi(uint8_t irq) {
    if(irq >= 8)
        io_port_out(IO_PORT_PIC2_CMD, IO_CMD_PIC_EOI);
    io_port_out(IO_PORT_PIC1_CMD, IO_CMD_PIC_EOI);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * pic_init() - Initialise the Programmable Interrupt Controller
 * @master_offset: Offset of all interrupts triggered by the master PIC.
 * @slave_offset:  Offset of all interrupts triggered by the slave PIC.
 *
 * Configures and initialises the legacy 8259 Programmable Interrupt Controller
 * by sending control words, and applies an offset to all IRQs raised by the
 * PIC, otherwise external interrupts will conflict with the range of
 * interrupts raised by the CPU itself during errors and the like.
 *
 * Return: E_SUCCESS
 */
int32_t pic_init(uint8_t master_offset, uint8_t slave_offset) {
    /* Begin PIC initialisation - set bit 4 to indicate this is ICW1, set bit
     * 0 to indicate an ICW4 will be sent later */
    klog("Sending ICW1_INIT to PIC1\n");
    io_port_out(IO_PORT_PIC1_CMD, IO_CMD_PIC_ICW1_INIT | IO_CMD_PIC_ICW1_ICW4);
    klog("Sending ICW1_INIT to PIC2\n");
    io_port_out(IO_PORT_PIC2_CMD, IO_CMD_PIC_ICW1_INIT | IO_CMD_PIC_ICW1_ICW4);

    /* Set master and slave PIC vector offset
     * We offset the interrupts so that they don't conflict with Intel's
     * reserved interrupts up to 0x1F */
    klog("Setting PIC offsets [PIC1: %d | PIC2: %d]\n",
           master_offset, slave_offset);
    io_port_out(IO_PORT_PIC1_DATA, master_offset);
    io_port_out(IO_PORT_PIC2_DATA, slave_offset);

    /* Configure slave PIC at IRQ2 */
    klog("Configuring slave PIC at IRQ2\n");
    io_port_out(IO_PORT_PIC1_DATA, IO_CMD_PIC_ICW3_MASTER);

    /* Configure slave PIC as secondary */
    klog("Configuring slave PIC as secondary\n");
    io_port_out(IO_PORT_PIC2_DATA, IO_CMD_PIC_ICW3_SLAVE);

    /* 8086 mode */
    klog("Enabling 8086 mode on PICs\n");
    io_port_out(IO_PORT_PIC1_DATA, IO_CMD_PIC_ICW4_8086_MODE);
    io_port_out(IO_PORT_PIC2_DATA, IO_CMD_PIC_ICW4_8086_MODE);

    /* Enable all IRQs */
    klog("Enabling all IRQs\n");
    io_port_out(IO_PORT_PIC1_DATA, 0x00);
    io_port_out(IO_PORT_PIC2_DATA, 0x00);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */
