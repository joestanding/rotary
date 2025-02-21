/*
 * arch/x86/kernel/task.c
 * x86-specific Task Implementation
 *
 * Manages x86-specific elements of tasks and task switching, such as managing
 * the TSS.
 */

#include <rotary/sched/task.h>
#include <arch/tss.h>

/* ------------------------------------------------------------------------- */

extern void isr_exit();
extern void task_context_switch(struct task * current_task,
                                struct task * next_task);

/* ------------------------------------------------------------------------- */

/**
 * arch_task_init() - Perform actions upon task subsystem initialisation.
 * @init_task: A pointer to the initial task (idle/init task)
 *
 * Currently empty.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t arch_task_init(struct task * init_task) {
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * arch_task_create() - Architecture-specific set-up for a new task.
 * @new_task: Pointer to the new task being created.
 *
 * Primarily constructs and assigns new page tables for the task depending
 * on its privilege level. Kernel tasks are assigned a pointer to the kernel
 * page directory, while user tasks are allocated individual page tables into
 * which the kernel is mapped.
 *
 * Initial stack values are also assigned to allow the task to be scheduled,
 * as state normally pushed by an interrupt (and expected to be popped during
 * scheduling) will not exist. These values provide a return address for
 * task_switch() and then a return address for the IRET instruction used to
 * switch execution to the scheduled task.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t arch_task_create(struct task * new_task) {

    klog("arch_task_create(): Init. task '%s'\n", new_task->name);

    /* Set-up the new kernel stack for the task. The kernel stack will need to
     * contain null-initialised register values that will be popped off the
     * stack by the task switching routine. */

    /* The return address saved prior to calling task_switch() will be
     * overwritten with a call to isr_exit(), which will restore the following
     * registers just like the end of any other ISR. As a result, the EIP is
     * where our task will start. */
    new_task->kstack_top -= sizeof(struct isr_registers);
    printk(LOG_TRACE, "arch_task_create(): isr_registers struct is at 0x%x\n",
           new_task->kstack_top);
    struct isr_registers * registers = \
        (struct isr_registers*)new_task->kstack_top;
    memset(registers, 0, sizeof(struct isr_registers));
    registers->eip = (uint32_t)new_task->start_addr;

    /* Configure the registers that will be "restored" by the iret. The value
     * of the code and data segment selectors will also define whether we
     * return into kernel mode or user mode. */
    if(new_task->type == TASK_KERNEL) {
        registers->cs = GDT_KERNEL_CODE_OFFSET | RPL_KERNELMODE;
        registers->ds = GDT_KERNEL_DATA_OFFSET | RPL_KERNELMODE;
        registers->eflags = EFLAGS_INTERRUPTS_ON | EFLAGS_IOPL_KERNELMODE;
        registers->user_ss = GDT_KERNEL_DATA_OFFSET | RPL_KERNELMODE;
    } else {
        registers->cs = GDT_USER_CODE_OFFSET | RPL_USERMODE;
        registers->ds = GDT_USER_DATA_OFFSET | RPL_USERMODE;
        registers->eflags = EFLAGS_INTERRUPTS_ON | EFLAGS_IOPL_USERMODE;
        registers->user_ss = GDT_USER_DATA_OFFSET | RPL_USERMODE;
        registers->user_esp = 0x440000;
    }

    /* This struct stores registers that are saved and restored by
     * task_context_switch(), EDI->EBP are restored via pops, EIP is
     * restored by ret */
    new_task->kstack_top -= sizeof(struct task_context);
    printk(LOG_TRACE, "arch_task_create(): context struct is at 0x%x\n",
           new_task->kstack_top);
    struct task_context * context = (struct task_context*)new_task->kstack_top;
    memset(context, 0, sizeof(struct task_context));
    context->edi = 0;
    context->esi = 0;
    context->ebx = 0;
    context->ebp = 0;
    context->eip = &isr_exit;

    klog("arch_task_create(): Complete, final values are:\n");
    klog("                    reg->eip: 0x%x\n", registers->eip);
    klog("                    ctx->eip: 0x%x\n", context->eip);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * arch_task_switch() - Perform a x86-specific task switch.
 * @curr_task: A pointer to the task structure for the currently executing
 *             task.
 * @next_task: A pointer to the task structure for task we're switching to.
 *
 * Updates the Task State Segment's ESP0 field to point to the bottom of the
 * next task's kernel stack, so the CPU can switch to the appropriate kernel
 * stack when an interrupt occurs during execution of a user-mode task.
 *
 * Performs the context switch by calling the `task_context_switch` assembly
 * routine, which restores the stack pointer of the next task. Execution of
 * the next task begins when the CPU pops the saved EIP value from the stack
 * upon IRET.
 *
 * Return: E_SUCCESS
 */
int32_t arch_task_switch(struct task * curr_task, struct task * next_task) {
    cpu_get_local()->tss.esp0 = (uint32_t)next_task->kstack_bot;
    task_context_switch(curr_task, next_task);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */
