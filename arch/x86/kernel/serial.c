/*
 * arch/x86/kernel/serial.c
 * x86 Serial I/O
 *
 * Initialise and handle serial input and output.
 */

#include <rotary/drivers/tty/serial.h>

/* Core sections contain definitions that must be implemented by all
 * architectures. Architecture-specific definitions have their own sections. */

/* ------------------------------------------------------------------------- */
/* Core Functions                                                            */
/* ------------------------------------------------------------------------- */

/**
 * arch_serial_init() - Architecture-specific serial initialisation.
 *
 * Assigns the default debug serial output port and initialises COM1 and COM2.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t arch_serial_init() {
    serial_set_debug_port(IO_PORT_SERIAL_COM2);
    if(!SUCCESS(x86_serial_init_port(IO_PORT_SERIAL_COM1)) ||
       !SUCCESS(x86_serial_init_port(IO_PORT_SERIAL_COM2))) {
        return E_ERROR;
    } else {
        return E_SUCCESS;
    }

}

/* ------------------------------------------------------------------------- */

/**
 * arch_serial_write_line() - Architure-specific serial write line.
 * @port: The serial port to write to.
 * @line  A pointer to a null-terminated string to write.
 *
 * Iterates through the provided line until a null terminator is found,
 * and writes to the relevant serial I/O port once the transmit buffer is
 * empty and ready for use.
 *
 * Return: E_SUCCESS
 */
int32_t arch_serial_write_line(uint32_t port, char * line) {
    while(*line != '\0') {
        while(x86_serial_is_transmit_empty(port) == false);
        io_port_out(port, line[0]);
        line++;
    }

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */
/* Architecture-specific Functions                                           */
/* ------------------------------------------------------------------------- */

/**
 * x86_serial_init_port() - Initialise a serial port.
 * @port: The numerical ID of the port to be initialised.
 *
 * Configures a serial port with the desired baud rate and other default
 * attributes through configuration of its I/O registers.
 *
 * Return: E_SUCCESS
 */
int32_t x86_serial_init_port(uint32_t port) {
    // The Interrupt Enable Register is used to enable or disable various
    // serial interrupts.
    // We set the register to zero to disable all interrupts.
    io_port_out(port + UART_IER, 0x00);

    // We set bit 7 of the Line Control Register to enable the Divisor Latch
    // Access Bit. When DLAB is set to '0' or '1' a number of registers are
    // changed. It is necessary as UART has 12 registers (including the work
    // register) while using only 8 port addresses.
    io_port_out(port + UART_LCR, UART_LCR_DLAB);

    // Now DLAB is enabled, we can configure the baud rate.
    io_port_out(port + 0, UART_FCR_BAUD_38400_LOW);
    io_port_out(port + 1, UART_FCR_BAUD_38400_HIGH);

    // The Line Control Register sets the general connection parameters.
    io_port_out(port + UART_LCR, UART_LCR_WORD_LENGTH_8B |
                                 UART_LCR_STOP_BIT_1 |
                                 UART_LCR_PARITY_NONE |
                                 UART_LCR_BREAK_DISABLE);

    // The FIFO Control Register is used to configure FIFO buffers.
    io_port_out(port + UART_FCR, UART_FCR_FIFO_ENABLE |
                                 UART_FCR_FIFO64_DISABLE |
                                 UART_FCR_CLEAR_RX_FIFO |
                                 UART_FCR_CLEAR_TX_FIFO |
                                 UART_FCR_INT_TRIG_14B);

    // Enable Force Data Terminal Ready, Force Request to Send
    io_port_out(port + UART_MCR, UART_MCR_FORCE_DTR |
                                 UART_MCR_FORCE_RTS |
                                 UART_MCR_AUX_OUTPUT_2);

    // Set in loopback mode, test the serial chip
    io_port_out(port + UART_MCR, UART_MCR_FORCE_RTS |
                                 UART_MCR_AUX_OUTPUT_1 |
                                 UART_MCR_AUX_OUTPUT_2 |
                                 UART_MCR_LOOPBACK_MODE);

    // Test that the serial port is in working order
    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    io_port_out(port + 0, 0xAE);
    if(io_port_in(port) != 0xAE) {
        return 1;
    }

    // Put the serial port into normal operation by disabling loopback,
    // enabling IRQs, and enabling outputs 1 and 2.
    io_port_out(port + UART_MCR, UART_MCR_FORCE_DTR |
                                 UART_MCR_FORCE_RTS |
                                 UART_MCR_AUX_OUTPUT_1 |
                                 UART_MCR_AUX_OUTPUT_2);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * x86_serial_is_transmit_empty() - Checks whether transmit buffer is empty.
 * @port: The numerical identifier for the port to be checked.
 *
 * Provides the ability to ensure that the transmit buffer is empty before
 * we start writing further bytes to it.
 *
 * Return: 1 if the transmit buffer is empty, 0 if it isn't.
 */
int32_t x86_serial_is_transmit_empty(uint32_t port) {
    uint32_t thre = io_port_in(port + UART_LSR) & UART_LSR_TX_HOLDING_EMPTY;
    if(thre == 0x20) {
        return 1;
    } else {
        return 0;
    }
}

/* ------------------------------------------------------------------------- */
