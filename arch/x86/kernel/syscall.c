/*
 * arch/x86/kernel/syscall.c
 * Syscall Handler
 */

#include <rotary/syscall.h>

/* ------------------------------------------------------------------------- */

/**
 * syscall_handler() - x86-specific syscall handler.
 * @regs: Pointer to a struct containing register values at interrupt.
 *
 * Handles syscalls from user-space tasks. Dispatches to the relevant syscall
 * handler depending on the value of the syscall number and other registers.
 */
void syscall_handler(struct syscall_regs * regs) {
    struct task * current_task = task_get_current();
    klog("Syscall from task ('%s' | ID: %d)\n",
            current_task->name,
            current_task->id);

    /*
    klog("EBX: 0x%x\n", regs->ebx);
    klog("ECX: 0x%x\n", regs->ecx);
    klog("EDX: 0x%x\n", regs->edx);
    klog("ESI: 0x%x\n", regs->esi);
    klog("EDI: 0x%x\n", regs->edi);
    klog("EBP: 0x%x\n", regs->ebp);
    klog("DS:  0x%x\n", regs->ds);
    klog("ES:  0x%x\n", regs->es);
    klog("FS:  0x%x\n", regs->fs);
    klog("GS:  0x%x\n", regs->gs);
    */

    switch(regs->syscall_no) {
        case SYSCALL_WRITE:
            syscall_write(regs->edi, (void*)regs->esi, regs->edx);
            break;
        default:
            klog("Invalid syscall: %d\n", regs->syscall_no);
            break;
    }
}

/* ------------------------------------------------------------------------- */
