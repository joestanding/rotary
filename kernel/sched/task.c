/*
 * kernel/task.c
 * Kernel and User Tasks
 *
 * Provides tasks for kernel and user-space multitasking.
 */

#include <rotary/sched/task.h>

/* ------------------------------------------------------------------------- */

extern void * KERNEL_STACK_TOP;
extern void * KERNEL_STACK_BOTTOM;

volatile atomic_flag task_lock = ATOMIC_FLAG_INIT;
uint32_t last_task_id = 1;
struct task   task_head;

/* ------------------------------------------------------------------------- */

/**
 * task_init() - Initialise task subsystem.
 *
 * Sets up the initial task using the current thread of execution, and invokes
 * any architecture-specific set-up that may also be desired. Once complete,
 * enables the task scheduler.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t task_init() {
    /* Our initial task will continue the execution of kernel_main(), as when
     * we switch to a new task the current EIP/ESP will be stored in the
     * tasks[0] struct. */

    /* Create list head */
    memset(&task_head, 0, sizeof(struct task));
    llist_init(&task_head.list_node);

    /* Allocate memory for the idle task and set default state */
    klog("Creating initial task\n");
    struct task * idle_task = kmalloc(sizeof(struct task));
    if(!idle_task) {
        PANIC("Failed to kmalloc() memory for initial task struct!\n");
        return E_ERROR;
    }

    /* Set default idle task state */
    idle_task->state = TASK_STATE_RUNNING;
    idle_task->id    = 0;

    /* Set the kernel stack bottom using symbol from the init stub */
    idle_task->kstack_bot = PHY_TO_VIR(&KERNEL_STACK_BOTTOM);

    /* Create a new virtual address space, by default it will include mappings
     * for the kernel */
    idle_task->vm_space = vm_space_new();

    /* Assign the task name */
    strncpy(idle_task->name, "kernel_idle", TASK_NAME_LENGTH_MAX);
    cpu_get_local()->current_task = idle_task;

    /* Add the idle task to the scheduler's task list */
    task_add_to_scheduler(idle_task);

    /* Perform any architecture-specific task system init. necessary */
    arch_task_init(cpu_get_local()->current_task);

    /* Enable the scheduler (otherwise PIT interrupts may trigger it before
     * it's ready */
    klog("Enabling scheduler\n");
    task_enable_scheduler();

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * task_create() - Create a new task.
 * @name:       The new task's name in ASCII format, with a maximum length of
 *              TASK_NAME_LENGTH_MAX, including the null terminator.
 * @type:       Specifies whether the task operates in kernel mode or user mode
 * @start_addr: The memory address where the task will begin execution.
 * @priority:   Task scheduling priority. Higher priority tasks receive more
 *              CPU time.
 * @state:      The initial state of the task, such as TASK_STATE_PAUSED to
 *              create a paused task.
 *
 * Allocates and initializes a new task structure with the specified parameters.
 * Sets up the kernel stack, address space, and architecture-specific data.
 * Adds the task to the scheduler's task list.
 *
 * Return:      Pointer to the new struct task object on success, NULL on failure.
 */

struct task * task_create(char * name, uint32_t type, void * start_addr,
                     uint32_t priority, uint32_t state) {

    klog("task_create(): Creating task %d (name: %s "
           "addr: 0x%x)\n", last_task_id, name, start_addr);

    /* Ensure valid task state is provided */
    if(state != TASK_STATE_PAUSED && state != TASK_STATE_WAITING) {
        klog("task_create(): Invalid starting state, can only be "
               "WAITING or PAUSED!\n");
        return NULL;
    }

    /* Ensure valid priority is provided */
    if(priority < TASK_PRIORITY_MIN || priority > TASK_PRIORITY_MAX) {
        klog("task_create(): Invalid priority (%d)! Must be "
               "between %d and %d.\n", priority, TASK_PRIORITY_MIN,
               TASK_PRIORITY_MAX);
        return NULL;
    }

    /* Take exclusive control of the task structure */
    lock(&task_lock);

    /* Allocate memory for the new task structure */
    uint32_t task_id  = last_task_id;
    struct task * new_task = kmalloc(sizeof(struct task));
    if(!new_task) {
        klog("task_create(): Failed to allocate memory for new task!\n");
        unlock(&task_lock);
        return NULL;
    }

    /* Update task structure with new task ID and name, and ensure the struct's
     * memory is cleared prior to use. */
    memset(new_task, 0, sizeof(struct task));
    new_task->id         = task_id;
    new_task->type       = type;
    new_task->start_addr = start_addr;
    new_task->state      = state;
    llist_init(&new_task->list_node);
    atomic_fetch_add(&last_task_id, 1);
    strncpy(new_task->name, name, TASK_NAME_LENGTH_MAX);

    /* Allocate the kernel stack for the new task */
    if(!SUCCESS(task_create_kernel_stack(new_task))) {
        klog("task_create(): Failed to allocate kernel stack for task '%s'!\n",
             name);
        unlock(&task_lock);
        return NULL;
    }

    /* Initialise the address space and page table, kernel mappings will be
     * included by default */
    new_task->vm_space = vm_space_new();
    if(!new_task->vm_space) {
        klog("task_create(): Failed to create virtual address space for task "
             "'%s'!\n", name);
        unlock(&task_lock);
        return NULL;
    }

    /* Perform architecture-specific task initialisation
     * This will include operations such as setting up arch-specific structures
     * on the stack, to be used in task switching code */
    if(!SUCCESS(arch_task_create(new_task))) {
        klog("task_create(): Arch-specific task create failed!\n");
        unlock(&task_lock);
        return NULL;
    }

    /* Add the task to the global task list */
    llist_add(&task_head.list_node, &new_task->list_node);

    /* Relinquish control of the task structure */
    unlock(&task_lock);

    klog("\n");
    klog("--- Task Created: '%s' (PID: %d) ---\n", new_task->name, task_id);
    if(new_task->type == TASK_KERNEL) {
        klog("  Type: Kernel Task\n");
    }
    if(new_task->type == TASK_USERMODE) {
        klog("  Type: Usermode Task\n");
    }
    klog("  KStackTop:  0x%x\n", new_task->kstack_top);
    klog("  KStackBot:  0x%x\n", new_task->kstack_bot);
    klog("  KStackSize: %d\n",   new_task->kstack_size);
    klog("  StartAddr:  0x%x\n", new_task->start_addr);
    klog("  ArchData:   0x%x\n", new_task->arch_data);
    klog("\n");

    return new_task;
}

/* ------------------------------------------------------------------------- */

/**
 * task_create_kernel_stack() - Allocate the kernel stack for a task.
 * @new_task: Pointer to the task for which the kernel stack is being created.
 *
 * Allocates memory for the kernel stack using page_alloc(). Initializes the
 * stack pointers (top and bottom) and clears the allocated memory.
 *
 * Return: E_SUCCESS on successful allocation, E_ERROR on failure.
 */
int32_t task_create_kernel_stack(struct task * new_task) {
    struct page * page = page_alloc(TASK_KERNEL_STACK_ORDER, 0);

    new_task->kstack_size = (1U << TASK_KERNEL_STACK_ORDER) * PAGE_SIZE;
    new_task->kstack_bot  = PAGE_VA(page) + new_task->kstack_size;
    new_task->kstack_top  = new_task->kstack_bot;

    memset((void*)PAGE_VA(page), 0, new_task->kstack_size);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * task_create_descriptors() - Set-up default file descriptors for a new task.
 * @new_task: Pointer to the target task.
 *
 * Set up the standard file descriptors such as STDIN and STDOUT for a newly
 * created task.
 *
 * Return: E_SUCCESS if success, E_ERROR if failure.
 */
/*
int32_t task_create_descriptors(struct task * new_task) {
    // Allocate and clear memory for the descriptor list dummy head
    new_task->descriptors = (descriptor*)kmalloc(sizeof(descriptor));
    if(!SUCCESS(new_task->descriptors)) {
        klog("Failed to alloc memory for descriptor "
               "dummy head!\n");
        return E_ERROR;
    }
    memset(new_task->descriptors, 0, sizeof(descriptor));

    // Set up the dummy head
    llist_init(&new_task->descriptors[0].list_entry);
    new_task->descriptors[0].type = DESCRIPTOR_TYPE_INVALID;

    // Allocate and clear memory for the default TTY descriptors
    descriptor * default_tty_read  =
        (descriptor*)kmalloc(sizeof(descriptor));
    descriptor * default_tty_write =
        (descriptor*)kmalloc(sizeof(descriptor));
    if(!SUCCESS(default_tty_read) || !SUCCESS(default_tty_write)) {
        klog("Failed to alloc memory for default TTY "
               "descriptors!\n");
        return E_ERROR;
    }
    memset(default_tty_read, 0, sizeof(descriptor));
    memset(default_tty_write, 0, sizeof(descriptor));

    default_tty_read->type     = DESCRIPTOR_TYPE_TTY;
    default_tty_read->access   = DESCRIPTOR_ACCESS_READ;
    default_tty_read->resource = tty_get_default();
    default_tty_write->type     = DESCRIPTOR_TYPE_TTY;
    default_tty_write->access   = DESCRIPTOR_ACCESS_WRITE;
    default_tty_write->resource = tty_get_default();


    llist_add_after(&new_task->descriptors[0].list_entry,
                    &default_tty_read->list_entry);
    llist_add_after(&default_tty_read->list_entry,
                    &default_tty_write->list_entry);

    return E_SUCCESS;
}
*/

/* ------------------------------------------------------------------------- */

/**
 * task_kill() - Mark a task as killed.
 * @task_id: ID of the task to kill.
 *
 * Mark a task as killed, which will prevent the scheduler from selecting this
 * task for future execution. The task will not immediately be purged: this
 * is done by task_purge() which is called by the scheduler.
 *
 * Return: E_SUCCESS on success, E_ERROR on failure.
 */
int32_t task_kill(uint32_t task_id) {
    if(task_id == 0) {
        klog("task_kill(): You cannot kill the idle process "
               "(ID 0)!\n", task_id);
        return E_ERROR;
    }

    struct task * task_tk = task_get_from_id(task_id);

    if(task_tk == NULL) {
        klog("task_kill(): No task with ID %d was found!\n", task_id);
        return E_ERROR;
    }

    if(task_tk->state == TASK_STATE_INVALID) {
        klog("task_kill(): Task ID %d is not an existing task!\n",
               task_id);
        return E_ERROR;
    }

    if(task_tk->state != TASK_STATE_RUNNING &&
       task_tk->state != TASK_STATE_WAITING &&
       task_tk->state != TASK_STATE_PAUSED) {
        klog("task_kill(): Task ID %d is not in a killable "
               "state!\n", task_id);
        return E_ERROR;
    }

    lock(&task_lock);

    klog("task_kill(): Killing task '%s' (ID: %d)..\n",
           task_tk->name, task_id);

    // Update state so we don't schedule this task anymore. The OS will
    // decide when to clear its resources such as allocated memory.
    task_tk->state = TASK_STATE_KILLED;

    unlock(&task_lock);

    klog("task_kill(): Marked task ID %d as killed\n", task_id);
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * task_purge() - Purge the specified task.
 * @task_id: The ID of the task to purge.
 *
 * Purges a task (most likely previously marked as killed) by resetting all
 * of the task attributes and de-allocating any allocated memory.
 *
 * Return: E_SUCCESS if success, E_ERROR if failure.
 */
int32_t task_purge(uint32_t task_id) {
    if(task_id == 0) {
        klog("task_purge(): You cannot purge the idle process "
               "(ID 0)!\n", task_id);
        return E_ERROR;
    }

    struct task * task_tk = task_get_from_id(task_id);

    if(task_tk == NULL) {
        klog("task_purge(): No task with ID %d could be found!\n", task_id);
        return E_ERROR;
    }

    if(task_tk->state != TASK_STATE_KILLED) {
        klog("task_purge(): Task ID %d is not a killed task!\n",
               task_id);
        return E_ERROR;
    }

    klog("task_purge(): Purging task ID %d\n", task_id);

    lock(&task_lock);

    /* Remove the task from the task list */
    llist_delete_node(&task_tk->list_node);

    /* Free memory allocated for its address space */
    vm_space_destroy(task_tk->vm_space);

    /* Free stack memory allocated for this task */
    void * kstack_start = task_tk->kstack_bot - task_tk->kstack_size;
    if(!SUCCESS(page_free_va(kstack_start, TASK_KERNEL_STACK_ORDER))) {
        klog("task_purge(): Failed to free task's kernel stack at 0x%x!\n",
             task_tk->kstack_bot);
    }

    /* Finally, free the task object itself */
    kfree(task_tk);

    unlock(&task_lock);

    klog("task_purge(): Purged task ID %d\n", task_id);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * task_exit_current() - Kill the currently running task.
 *
 * Marks the currently executing task as killed, so that the task will be
 * purged by the scheduler when ready.
 *
 * Return: E_SUCCESS if success, E_ERROR if failure.
 */
int32_t task_exit_current() {
    klog("task_exit(): Exiting task '%s'\n",
         cpu_get_local()->current_task->name);
    lock(&task_lock);
    cpu_get_local()->current_task->state = TASK_STATE_KILLED;
    unlock(&task_lock);
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * task_get_current() - Retrieve the currently executing task.
 *
 * This implementation will most likely need changing if we add support for
 * multiple execution cores in future.
 *
 * Return: A pointer to the currently executing task.
 */
struct task * task_get_current() {
    return NULL;
}

/* ------------------------------------------------------------------------- */

/**
 * task_get_from_id() - Retrieve a pointer to a task with a given ID.
 * @task_id:  THe ID of the task to retrieve.
 *
 * Return: A pointer to the specified task object.
 */
struct task * task_get_from_id(uint32_t task_id) {
    lock(&task_lock);

    struct task *task = NULL;
    list_for_each(task, &task_head.list_node, list_node) {  // Updated here
        if(task->id == task_id) {
            klog("task_get_from_id(%d): Found task '%s'\n",
                 task_id, task->name);
            unlock(&task_lock);
            return task;
        }
    }

    klog("task_get_from_id(): No task with ID %d was found!\n", task_id);
    unlock(&task_lock);
    return NULL;
}

/* ------------------------------------------------------------------------- */

/**
 * task_schedule() - Task scheduler.
 *
 * Currently an implementation of a round-robin scheduler, which simply picks
 * the next task from the list, with no consideration of priority. To be
 * updated with priority support.
 *
 * Invokes the architecture-specific task_switch() routine to conduct the
 * actual task switch.
 *
 * Return: None
 */
void task_schedule() {
    // Don't allow task switching before everything's set up
    if(!cpu_get_local()->sched_enabled)
        return;

    // Count CPU ticks for this task
    cpu_get_local()->current_task->ticks += 1;

    struct task *task_tp = NULL;
    list_for_each(task_tp, &task_head.list_node, list_node) {
        if(task_tp->state == TASK_STATE_KILLED) {
            klog("task_schedule(): Found KILLED task (%d) awaiting purge\n",
                 task_tp->id);
            task_purge(task_tp->id);
        }
    }

    /* Identify the next task, naive round-robin implementation for now */
    struct task * prev = cpu_get_local()->current_task;
    struct task * next = NULL;

    /* First, get a reference to the next list node */
    list_node_t * next_node = cpu_get_local()->current_task->list_node.next;
    struct task * curr_task = NULL;
    int task_found = 0;

    while(!task_found) {
        /* First, check if the next node is NULL - if it is, we wrap around
         * to the first in the list */
        if(next_node == NULL) {
            next_node = &task_head.list_node;
        }

        /* Now, check if the task is in a valid state to be executed */
        curr_task = (struct task *)container_of(next_node,
                                          struct task,
                                          list_node);

        if(curr_task->state == TASK_STATE_WAITING ||
           curr_task->state == TASK_STATE_RUNNING) {
            next = curr_task;
            task_found = 1;
        }
        /* Skip to the next task */
        next_node = next_node->next;

    }

    cpu_get_local()->current_task = next;

    // Update states
    if(prev->state == TASK_STATE_RUNNING) {
        prev->state = TASK_STATE_WAITING;
    }
    next->state = TASK_STATE_RUNNING;

    arch_task_switch(prev, next);
}

/* ------------------------------------------------------------------------- */

/**
 * task_print() - Print all existing task information.
 *
 * Currently prints all available task information, however this function
 * should be removed once this information can be exposed via a Unix-like
 * filesystem and read by a user-mode process.
 *
 * Return: None
 */
void task_print() {
    klog("Task List\n");
    klog("-----------------------------\n");


    lock(&task_lock);

    list_node_t * curr_node = task_head.list_node.next;
    struct task * task = NULL;
    while(curr_node != NULL) {
        task = (struct task *)container_of(curr_node,
                                     struct task,
                                     list_node);

        char state[16];
        memset(&state, 0x00, sizeof(state));

        if(task->state == TASK_STATE_INVALID)
            sprintf(state, "%s", "EMPTY");
        if(task->state == TASK_STATE_RUNNING)
            sprintf(state, "%s", "RUNNING");
        if(task->state == TASK_STATE_WAITING)
            sprintf(state, "%s", "WAITING");
        if(task->state == TASK_STATE_PAUSED)
            sprintf(state, "%s", "PAUSED");
        if(task->state == TASK_STATE_KILLED)
            sprintf(state, "%s", "KILLED");
        if(task->state != TASK_STATE_INVALID) {
            klog("[%d] '%s' (%s, priority %d) \n", task->id,
                   task->name, state, task->priority);
            if(task->type == TASK_KERNEL)
                klog("      type:        Kernel\n");
            if(task->type == TASK_USERMODE)
                klog("      type:        Usermode\n");
            klog("      kstack top:  0x%x | bot: 0x%x\n",
                   task->kstack_top, task->kstack_bot);
            klog("      kstack_size: %d bytes\n",
                             task->kstack_size);
            klog("      stack_used:  %d bytes\n",
                   task->kstack_bot - task->kstack_top);
            klog("      vm_space:    0x%x\n", task->vm_space);
            klog("      ticks:  %d\n", task->ticks);
        }

        curr_node = curr_node->next;
    }

    unlock(&task_lock);

    klog("\n");
}

/* ------------------------------------------------------------------------- */

/**
 * task_enable_scheduler() - Enable the task scheduler.
 */
void task_enable_scheduler() {
    cpu_get_local()->sched_enabled = 1;
}

/* ------------------------------------------------------------------------- */

/**
 * task_disable_scheduler() - Disable the task scheduler.
 */
void task_disable_scheduler() {
    cpu_get_local()->sched_enabled = 0;
}

/* ------------------------------------------------------------------------- */

void task_add_to_scheduler(struct task * new_task) {
    llist_add(&task_head.list_node, &new_task->list_node);
}

/* ------------------------------------------------------------------------- */
