/*
 * interrupts.c
 * x86 Interrupt Configuration and Handling
 *
 * Functions to enable/disable hardware interrupts, configure the Interrupt
 * Descriptor Table (IDT), register interrupts, and handle fired interrupts.
 */

#include <arch/interrupts.h>

void * interrupt_handlers[MAX_INTERRUPT_HANDLERS];
idt_gate_t idt[256];
idt_pointer_t idt_ptr;

/* ------------------------------------------------------------------------- */

/**
 * enable_hardware_interrupts() - Enable hardware interrupts
 *
 * Enables hardware interrupts by setting the Interrupt Flag via `STI`.
 */
void enable_hardware_interrupts() {
    asm("sti");
}

/* ------------------------------------------------------------------------- */

/**
 * disable_hardware_interrupts() - Disable hardware interrupts
 *
 * Disables hardware interrupts by clearing the Interrupt Flag via `CLI`.
 */
void disable_hardware_interrupts() {
    asm("cli");
}

/* ------------------------------------------------------------------------- */

/**
 * register_interrupt_handlers() - Register a handler for a given interrupt
 * @int_num:      The interrupt type that the handler wants to handle
 * @handler_addr: Pointer to the event handler function to exec. upon interrupt
 *
 * Registers an event handler for the specified interrupt number, that will be
 * called when the interrupt fires.
 */
void register_interrupt_handler(uint32_t int_num, void * handler_addr) {
    klog("Registered handler (0x%x) for int. %d\n", handler_addr,
           int_num);
    interrupt_handlers[int_num] = handler_addr;
}

/* ------------------------------------------------------------------------- */

/**
 * isr_handler() - Interrupt Service Routine handler.
 * @registers: Pointer to a struct containing all CPU registers at the time of
 *             the fired interrupt.
 *
 * Called from the Interrupt Service Routine assembly stub, which ensures
 * all necessary registers are pushed to the stack for access in this ISR
 * handler. Kernel handling of interrupts begins here.
 */
void isr_handler(struct isr_registers * registers) {

    /* Immediately send an End of Interrupt for a timer interrupt */
    if(registers->int_num == INT_PIT) {
        pic_send_eoi(registers->int_num);
    }

    /* If a handler for this interrupt is registered, call it */
    if(interrupt_handlers[registers->int_num] != NULL) {
        func * handler = (func*)interrupt_handlers[registers->int_num];
        handler(registers);
    }

    /* Print some debugging information for exceptions raised by the CPU */
    if(registers->int_num >= 0 && registers->int_num < 32) {

        if(registers->int_num == INT_PAGE_FAULT) {
            paging_handle_page_fault(registers);
            pic_send_eoi(registers->int_num);
            return;
        }

        if(registers->int_num == 0)
            printk(LOG_INFO, "[isr] Interrupt 0: Divide by Zero\n");
        if(registers->int_num == 1)
            printk(LOG_INFO, "[isr] Interrupt 1: Single Step\n");
        if(registers->int_num == 2)
            printk(LOG_INFO, "[isr] Interrupt 2: Non-Maskable (NMI)\n");
        if(registers->int_num == 3)
            printk(LOG_INFO, "[isr] Interrupt 3: Breakpoint\n");
        if(registers->int_num == 4)
            printk(LOG_INFO, "[isr] Interrupt 4: Overflow Trap\n");
        if(registers->int_num == 5)
            printk(LOG_INFO, "[isr] Interrupt 5: Bound Range Exceeded\n");
        if(registers->int_num == 6)
            printk(LOG_INFO, "[isr] Interrupt 6: Invalid Opcode\n");
        if(registers->int_num == 7)
            printk(LOG_INFO, "[isr] Interrupt 7: Coprocessor Not Available\n");
        if(registers->int_num == 8)
            printk(LOG_INFO, "[isr] Interrupt 8: Double Fault Exception\n");
        if(registers->int_num == 9)
            printk(LOG_INFO, "[isr] Interrupt 9: Coprocessor Segment "
                             "Overrun\n");
        if(registers->int_num == 10)
            printk(LOG_INFO, "[isr] Interrupt 10: Invalid Task State Segment "
                             "(TSS)\n");
        if(registers->int_num == 11)
            printk(LOG_INFO, "[isr] Interrupt 11: Segment Not Present\n");
        if(registers->int_num == 12)
            printk(LOG_INFO, "[isr] Interrupt 12: Stack Exception\n");
        if(registers->int_num == 13)
            printk(LOG_INFO, "[isr] Interrupt 13: General Protection "
                             "Exception\n");
        if(registers->int_num == 14)
            printk(LOG_INFO, "[isr] Interrupt 14: Page Fault\n");

        printk(LOG_INFO, "[isr] Error Code: %d\n", registers->error_code);
        printk(LOG_INFO, "Registers\n");
        printk(LOG_INFO, "---------\n");
        printk(LOG_INFO, "EAX:        0x%x | EBX:        0x%x\n",
               registers->eax, registers->ebx);
        printk(LOG_INFO, "ECX:        0x%x | EDX:        0x%x\n",
               registers->ecx, registers->edx);
        printk(LOG_INFO, "ESI:        0x%x | EDI:        0x%x\n",
               registers->esi, registers->edi);
        printk(LOG_INFO, "EIP:        0x%x | ESP:        0x%x\n",
               registers->eip, registers->esp);
        printk(LOG_INFO, "CS:         0x%x\n", registers->cs);
        printk(LOG_INFO, "EFLAGS:     0x%x\n", registers->eflags);

        if(registers->int_num == INT_PAGE_FAULT) {
            uint8_t present = registers->error_code & 0x1;
            uint8_t write = (registers->error_code >> 1) & 0x1;
            uint8_t user = (registers->error_code >> 2) & 0x1;
            uint8_t rsvd_write = (registers->error_code >> 3) & 0x1;
            uint8_t ins_fetch = (registers->error_code >> 4) & 0x1;
            uint8_t pkey = (registers->error_code >> 5) & 0x1;
            uint8_t shadow_stack = (registers->error_code >> 6) & 0x1;
            //uint8_t sgx = (registers->error_code >> 7) & 0x1;
            uint32_t cr2;
            __asm__ volatile("mov %%cr2, %0" : "=r" (cr2));

            printk(LOG_INFO, "Unhandled page fault at:   0x%x\n", cr2);


            if(present == 1) {
                printk(LOG_INFO, "The fault was caused by a page-level "
                       "protection violation.\n");
            } else {
                printk(LOG_INFO, "The fault was caused by a non-present "
                       "page.\n");
            }
            if(write) {
                printk(LOG_INFO, "The access causing the fault was a "
                       "write.\n");
            } else {
                printk(LOG_INFO, "The access causing the fault was a "
                       "read.\n");
            }
            if(user) {
                printk(LOG_INFO, "A user-mode access caused the fault.\n");
            } else {
                printk(LOG_INFO, "A supervisor-mode access caused the "
                       "fault.\n");
            }
            if(rsvd_write) {
                printk(LOG_INFO, "The fault was caused by a reserved bit set "
                       "to 1 in some page entry.\n");
            }
            if(ins_fetch) {
                printk(LOG_INFO, "The fault was caused by an instruction "
                       "fetch.\n");
            }
            if(pkey) {
                printk(LOG_INFO, "There was a protection-key violation.\n");
            }
            if(shadow_stack) {
                printk(LOG_INFO, "The fault was caused by a shadow-stack "
                       "access.\n");
            }
        }

        debug_break();
    }

    /* Send an End of Interrupt signal to the PIC */
    pic_send_eoi(registers->int_num);
}

/* ------------------------------------------------------------------------- */

/**
 * set_idt_gate() - Register an interrupt gate in the descriptor table
 * @int_num:     The interrupt type to register the gate for
 * @handle_addr: A pointer to the interrupt handler function
 * @dpl:         The minimum privilege level required to invoke the gate
 *
 * Registers an interrupt handler within the Interrupt Descriptor Table. This
 * is used to install the ISR handlers within the ISR handling assembly stub.
 */
void set_idt_gate(uint32_t int_num, uint32_t handle_addr, uint32_t dpl) {
    idt[int_num].low_offset = LOW_16(handle_addr);
    idt[int_num].segment_selector = GDT_KERNEL_CODE_OFFSET;
    idt[int_num].reserved = 0x00;
    idt[int_num].high_offset = HIGH_16(handle_addr);
    idt[int_num].gate_type = IDT_TYPE_32_INT;
    idt[int_num].s = 0x00;
    idt[int_num].dpl = dpl;
    idt[int_num].present = 0x01;
}


/* ------------------------------------------------------------------------- */

/**
 * idt_load() - Load the Interrupt Descriptor Table
 *
 * Return: E_SUCCESS
 */
int32_t idt_load() {
    klog("IDT addr: 0x%x, descriptor 0x%x\n", &idt, &idt_ptr);
    idt_ptr.base = (uint32_t)&idt;
    idt_ptr.limit = 256 * sizeof(idt_gate_t) - 1;
    asm("lidt (%0)" : : "r" (&idt_ptr));

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * idt_init() - Configures the Interrupt Descriptor Table
 *
 * Sets up the Interrupt Descriptor Table with necessary gates, allowing us to
 * handle all potential interrupts.
 *
 * The Descriptor Privilege Level (DPL) specifies the minimum CPL required
 * to access the interrupt via the `int` instruction. Hardware interrupts
 * ignore this mechanism.
 *
 * Return: E_SUCCESS
 */
int32_t idt_init() {
    /* CPU Exceptions and Interrupts */
    set_idt_gate(0, (uint32_t)isr0, IDT_DPL_KERNEL);
    set_idt_gate(1, (uint32_t)isr1, IDT_DPL_KERNEL);
    set_idt_gate(2, (uint32_t)isr2, IDT_DPL_KERNEL);
    set_idt_gate(3, (uint32_t)isr3, IDT_DPL_KERNEL);
    set_idt_gate(4, (uint32_t)isr4, IDT_DPL_KERNEL);
    set_idt_gate(5, (uint32_t)isr5, IDT_DPL_KERNEL);
    set_idt_gate(6, (uint32_t)isr6, IDT_DPL_KERNEL);
    set_idt_gate(7, (uint32_t)isr7, IDT_DPL_KERNEL);
    set_idt_gate(8, (uint32_t)isr8, IDT_DPL_KERNEL);
    set_idt_gate(9, (uint32_t)isr9, IDT_DPL_KERNEL);
    set_idt_gate(10, (uint32_t)isr10, IDT_DPL_KERNEL);
    set_idt_gate(11, (uint32_t)isr11, IDT_DPL_KERNEL);
    set_idt_gate(12, (uint32_t)isr12, IDT_DPL_KERNEL);
    set_idt_gate(13, (uint32_t)isr13, IDT_DPL_KERNEL);
    set_idt_gate(14, (uint32_t)isr14, IDT_DPL_KERNEL);
    set_idt_gate(15, (uint32_t)isr15, IDT_DPL_KERNEL);
    set_idt_gate(16, (uint32_t)isr16, IDT_DPL_KERNEL);
    set_idt_gate(17, (uint32_t)isr17, IDT_DPL_KERNEL);
    set_idt_gate(18, (uint32_t)isr18, IDT_DPL_KERNEL);
    set_idt_gate(19, (uint32_t)isr19, IDT_DPL_KERNEL);
    set_idt_gate(20, (uint32_t)isr20, IDT_DPL_KERNEL);
    set_idt_gate(21, (uint32_t)isr21, IDT_DPL_KERNEL);
    set_idt_gate(22, (uint32_t)isr22, IDT_DPL_KERNEL);
    set_idt_gate(23, (uint32_t)isr23, IDT_DPL_KERNEL);
    set_idt_gate(24, (uint32_t)isr24, IDT_DPL_KERNEL);
    set_idt_gate(25, (uint32_t)isr25, IDT_DPL_KERNEL);
    set_idt_gate(26, (uint32_t)isr26, IDT_DPL_KERNEL);
    set_idt_gate(27, (uint32_t)isr27, IDT_DPL_KERNEL);
    set_idt_gate(28, (uint32_t)isr28, IDT_DPL_KERNEL);
    set_idt_gate(29, (uint32_t)isr29, IDT_DPL_KERNEL);
    set_idt_gate(30, (uint32_t)isr30, IDT_DPL_KERNEL);
    set_idt_gate(31, (uint32_t)isr31, IDT_DPL_KERNEL);
    /* External IRQs */
    set_idt_gate(32, (uint32_t)isr32, IDT_DPL_KERNEL);
    set_idt_gate(33, (uint32_t)isr33, IDT_DPL_KERNEL);
    set_idt_gate(34, (uint32_t)isr34, IDT_DPL_KERNEL);
    set_idt_gate(35, (uint32_t)isr35, IDT_DPL_KERNEL);
    set_idt_gate(36, (uint32_t)isr36, IDT_DPL_KERNEL);
    set_idt_gate(37, (uint32_t)isr37, IDT_DPL_KERNEL);
    set_idt_gate(38, (uint32_t)isr38, IDT_DPL_KERNEL);
    set_idt_gate(39, (uint32_t)isr39, IDT_DPL_KERNEL);
    set_idt_gate(40, (uint32_t)isr40, IDT_DPL_KERNEL);
    set_idt_gate(41, (uint32_t)isr41, IDT_DPL_KERNEL);
    set_idt_gate(42, (uint32_t)isr42, IDT_DPL_KERNEL);
    set_idt_gate(43, (uint32_t)isr43, IDT_DPL_KERNEL);
    set_idt_gate(44, (uint32_t)isr44, IDT_DPL_KERNEL);
    set_idt_gate(45, (uint32_t)isr45, IDT_DPL_KERNEL);
    set_idt_gate(46, (uint32_t)isr46, IDT_DPL_KERNEL);
    set_idt_gate(47, (uint32_t)isr47, IDT_DPL_KERNEL);
    /* Syscall */
    set_idt_gate(64, (uint32_t)isr64, IDT_DPL_USER);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */
