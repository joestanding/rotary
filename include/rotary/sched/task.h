/*
 * include/rotary/sched/task.h
 * Task Management
 */

#ifndef INC_SCHED_TASK_H
#define INC_SCHED_TASK_H

#include <rotary/core.h>
#include <rotary/panic.h>
#include <rotary/debug.h>
#include <rotary/util/math.h>
#include <rotary/logging.h>
#include <rotary/mm/kmalloc.h>
#include <rotary/mm/ptable.h>
#include <rotary/mm/vm.h>
#include <rotary/sync.h>
#include <rotary/core/descriptor.h>
#include <rotary/drivers/tty/tty.h>
#include <arch/task.h>

/* ------------------------------------------------------------------------- */

/* States */
#define TASK_STATE_INVALID      0
#define TASK_STATE_RUNNING      1
#define TASK_STATE_WAITING      2
#define TASK_STATE_PAUSED       4
#define TASK_STATE_KILLED       5

/* Limits */
#define TASK_MAX                8
#define TASK_DESCRIPTOR_MAX     1024
#define TASK_NAME_LENGTH_MAX    16

/* Privilege Levels */
#define TASK_KERNEL             0
#define TASK_USERMODE           1

/* Priorities */
#define TASK_PRIORITY_MIN       1
#define TASK_PRIORITY_MAX       5

/* Size */
#define TASK_KERNEL_STACK_SIZE  (PAGE_SIZE * 2)
#define TASK_KERNEL_STACK_ORDER 4

/* ------------------------------------------------------------------------- */

struct vm_space;

/* ------------------------------------------------------------------------- */

/* NOTE: If updated, the offsets in task-switch.asm must also be updated! */
struct task {
    /* Basic information */
    uint32_t id;
    uint32_t type;
    uint32_t state;
    uint32_t priority;
    uint32_t ticks;

    /* Kernel stack */
    void *   kstack_top;
    void *   kstack_bot;
    uint32_t kstack_size;

    /* Memory address space information */
    struct vm_space * vm_space;

    /* Entry point */
    void *   start_addr;

    /* Task list entry */
    list_node_t list_node;

    char name[TASK_NAME_LENGTH_MAX];

    /* Architecture-specific CPU info, e.g. pointer to the TSS on x86 */
    struct arch_data * arch_data;
};

/* cpu_info relies on struct task */
#include <arch/cpu.h>

/* ------------------------------------------------------------------------- */

int32_t  task_init();
struct task * task_create(char * name, uint32_t type, void * start_addr,
                     uint32_t priority, uint32_t state);

int32_t  task_create_kernel_stack(struct task * new_task);
int32_t  task_create_descriptors(struct task * new_task);

int32_t  task_kill(uint32_t task_id);
int32_t  task_purge(uint32_t task_id);
int32_t  task_exit_current();
struct task * task_get_current();
struct task * task_get_from_id(uint32_t task_id);
void     task_schedule();
void     task_print();
void     task_enable_scheduler();
void     task_disable_scheduler();
void     task_add_to_scheduler(struct task * new_task);

/* ------------------------------------------------------------------------- */

int32_t  arch_task_init(struct task * init_task);
int32_t  arch_task_create(struct task * new_task);
int32_t  arch_task_switch(struct task * curr_task, struct task * next_task);

/* ------------------------------------------------------------------------- */

#endif
