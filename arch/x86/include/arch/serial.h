/*
 * arch/x86/include/arch/serial.h
 * x86 Serial I/O
 */

#ifndef INC_ARCH_SERIAL_H
#define INC_ARCH_SERIAL_H

#include <arch/io_port.h>

/* ------------------------------------------------------------------------- */

/* UART Register Offsets */
#define UART_RX		0	/* In:  Receive buffer (DLAB=0) */
#define UART_TX		0	/* Out: Transmit buffer (DLAB=0) */
#define UART_DLL	0	/* Out: Devisor Latch Low (DLAB=1) */
#define UART_DLM	1	/* Out: Devisor Latch High (DLAB=1) */
#define UART_IER	1	/* Out: Interrupt Enable Register */
#define UART_IIR	2	/* In:  Interrupt ID Register */
#define UART_FCR	2	/* Out: FIFO Control Register */
#define UART_LCR	3	/* Out: Line Control Register */
#define UART_MCR	4	/* Out: Modem Control Register */
#define UART_LSR	5	/* In:  Line Status Register */
#define UART_MSR	6	/* In:  Modem Status Register */
#define UART_SCR	7	/* I/O: Scratch Register */

/* Line Status Register */
#define UART_LSR_DATA_READY         0x01
#define UART_LSR_OVERRUN_ERROR      0x02
#define UART_LSR_PARITY_ERROR       0x04
#define UART_LSR_FRAMING_ERROR      0x08
#define UART_LSR_BREAK_INDICATOR    0x10
#define UART_LSR_TX_HOLDING_EMPTY   0x20
#define UART_LSR_TX_EMPTY           0x40
#define UART_LSR_IMPENDING_ERROR    0x80

/* Line Control Register */
#define UART_LCR_DLAB               0x80
#define UART_LCR_BREAK_ENABLE       0x40
#define UART_LCR_BREAK_DISABLE      0x00
#define UART_LCR_PARITY_LOW         0x38
#define UART_LCR_PARITY_HIGH        0x28
#define UART_LCR_PARITY_EVEN        0x18
#define UART_LCR_PARITY_ODD         0x08
#define UART_LCR_PARITY_NONE        0x00
#define UART_LCR_STOP_BIT_1         0x00
#define UART_LCR_STOP_BIT_2         0x04
#define UART_LCR_WORD_LENGTH_5B     0x00
#define UART_LCR_WORD_LENGTH_6B     0x01
#define UART_LCR_WORD_LENGTH_7B     0x02
#define UART_LCR_WORD_LENGTH_8B     0x03

/* Modem Control Register */
#define UART_MCR_FORCE_DTR          0x01
#define UART_MCR_FORCE_RTS          0x02
#define UART_MCR_AUX_OUTPUT_1       0x04
#define UART_MCR_AUX_OUTPUT_2       0x08
#define UART_MCR_LOOPBACK_MODE      0x10
#define UART_MCR_AUTOFLOW_CTL       0x20

/* FIFO Control Register */
#define UART_FCR_BAUD_50_HIGH       0x09
#define UART_FCR_BAUD_50_LOW        0x00
#define UART_FCR_BAUD_300_HIGH      0x01
#define UART_FCR_BAUD_300_LOW       0x80
#define UART_FCR_BAUD_600_HIGH      0x00
#define UART_FCR_BAUD_600_LOW       0xC0
#define UART_FCR_BAUD_2400_HIGH     0x00
#define UART_FCR_BAUD_2400_LOW      0x30
#define UART_FCR_BAUD_4800_HIGH     0x00
#define UART_FCR_BAUD_4800_LOW      0x18
#define UART_FCR_BAUD_9600_HIGH     0x00
#define UART_FCR_BAUD_9600_LOW      0x0C
#define UART_FCR_BAUD_19200_HIGH    0x00
#define UART_FCR_BAUD_19200_LOW     0x06
#define UART_FCR_BAUD_38400_HIGH    0x00
#define UART_FCR_BAUD_38400_LOW     0x03
#define UART_FCR_BAUD_57600_HIGH    0x00
#define UART_FCR_BAUD_57600_LOW     0x02
#define UART_FCR_BAUD_115200_HIGH   0x00
#define UART_FCR_BAUD_115200_LOW    0x01

#define UART_FCR_FIFO_DISABLE       0x00
#define UART_FCR_FIFO_ENABLE        0x01

#define UART_FCR_CLEAR_RX_FIFO      0x02
#define UART_FCR_CLEAR_TX_FIFO      0x04

#define UART_FCR_INT_TRIG_1B        0x00
#define UART_FCR_INT_TRIG_4B        0x40
#define UART_FCR_INT_TRIG_8B        0x80
#define UART_FCR_INT_TRIG_14B       0xC0

#define UART_FCR_FIFO64_DISABLE     0x00
#define UART_FCR_FIFO64_ENABLE      0x20

#define UART_FCR_DMA_MODE_SEL       0x08

/* ------------------------------------------------------------------------- */

int32_t x86_serial_init_port(uint32_t port);
int32_t x86_serial_is_transmit_empty(uint32_t port);

/* ------------------------------------------------------------------------- */

#endif
