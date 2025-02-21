/*
 * arch/x86/include/arch/cpu.h
 * Per-CPU State
 */

#ifndef INC_ARCH_CPU_H
#define INC_ARCH_CPU_H

#include <rotary/sched/task.h>
#include <arch/tss.h>
#include <arch/gdt.h>
#include <arch/msr.h>

/* ------------------------------------------------------------------------- */

#define EFLAGS_INTERRUPTS_ON   0x200
#define EFLAGS_INTERRUPTS_OFF  0x00

#define EFLAGS_IOPL_KERNELMODE 0x00
#define EFLAGS_IOPL_USERMODE   0x3000

/* ------------------------------------------------------------------------- */

struct task;

/* ------------------------------------------------------------------------- */

struct cpu_info {
    uint16_t cpu_id;
    struct task * current_task;
    uint8_t sched_enabled;
    struct tss tss;
    __attribute__((aligned(8))) gdt_descriptor_t gdt_desc;
    __attribute__((aligned(8))) gdt_entry_t gdt_entries[GDT_ENTRY_COUNT];
    struct cpu_info * self;
};

/* ------------------------------------------------------------------------- */

int32_t cpu_init();
void    cpu_init_gdt(struct cpu_info * cpu);
void    cpu_load_gdtr(gdt_descriptor_t * gdtr);
void    cpu_set_gs(uint16_t value);
struct  cpu_info * cpu_get_local();

/* ------------------------------------------------------------------------- */

#endif
