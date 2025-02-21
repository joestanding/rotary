/*
 * arch/x86/include/arch/interrupts.h
 * x86 Interrupts
 */

#ifndef INC_ARCH_INTERRUPTS_H
#define INC_ARCH_INTERRUPTS_H

#include <rotary/core.h>
#include <rotary/debug.h>
#include <rotary/logging.h>
#include <arch/paging.h>
#include <arch/pic8259.h>
#include <arch/gdt.h>

/* ------------------------------------------------------------------------- */

#define MAX_INTERRUPT_HANDLERS 256

/* CPU-raised exceptions */
#define INT_DIV_BY_ZERO         0
#define INT_SINGLE_STEP         1
#define INT_NMI                 2
#define INT_BREAKPOINT          3
#define INT_OVERFLOW            4
#define INT_BOUND_EXCEEDED      5
#define INT_INVALID_OPCODE      6
#define INT_FPU_NOT_AVAIL       7
#define INT_DOUBLE_FAULT        8
#define INT_COPRO_SEG_ORUN      9
#define INT_INVALID_TSS         10
#define INT_SEG_NOT_PRESENT     11
#define INT_STACK_SEG_FAULT     12
#define INT_GENERAL_PROT_FAULT  13
#define INT_PAGE_FAULT          14
#define INT_RESERVED            15
#define INT_X87_FPU_EXCEPTION   16
#define INT_ALIGNMENT_CHECK     17
#define INT_MACHINE_CHECK       18
#define INT_SIMD_FPU_EXCEPTION  19
#define INT_VIRT_EXCEPTION      20
#define CONTROL_PROT_EXCEPTION  21

/* External interrupts */
#define INT_PIT                 32
#define INT_KEYBOARD            33

/* Software interrupts */
#define INT_SYSCALL             64

/* IDT gate types */
#define IDT_TYPE_TASK           0x05
#define IDT_TYPE_16_INT         0x06
#define IDT_TYPE_16_TRAP        0x07
#define IDT_TYPE_32_INT         0x0E
#define IDT_TYPE_32_TRAP        0x0F

/* IDT DPLs */
#define IDT_DPL_KERNEL          0x00
#define IDT_DPL_USER            0x03

/* ------------------------------------------------------------------------- */

#define LOW_16(address) (uint16_t)((address) & 0xFFFF)
#define HIGH_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

/* ------------------------------------------------------------------------- */

/* Represents each IDT gate in the IDT table */
typedef struct {
    uint16_t low_offset;
    uint16_t segment_selector;
    uint8_t  reserved;
    uint8_t gate_type: 4;
    uint8_t s: 1;
    uint8_t dpl: 2;
    uint8_t present: 1;
    uint16_t high_offset;
} __attribute__((packed)) idt_gate_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_pointer_t;

/* Contains information passed through to the generic ISR handler
 * from the ISR assembly stub routine */
struct isr_registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_num, error_code;
    uint32_t eip, cs, eflags, user_esp, user_ss;
};

typedef void func(struct isr_registers * registers);

/* ------------------------------------------------------------------------- */

void    set_idt_gate(uint32_t int_num, uint32_t handle_addr, uint32_t dpl);
void    enable_hardware_interrupts();
void    disable_hardware_interrupts();
void    register_interrupt_handler(uint32_t int_num, void * handler_addr);

int32_t idt_load();
int32_t idt_init();

/* Called by the generic ISR stub routine in isr.asm */
void isr_handler(struct isr_registers * registers);

/* ------------------------------------------------------------------------- */

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();

/* Syscall */
extern void isr64();

/* ------------------------------------------------------------------------- */

#endif
