/*
 * kernel/core/shell.c
 * Kernel Shell
 *
 * Temporary shell implementation until I have a user-space shell up.
 */

#include <rotary/core/shell.h>

char shell_prompt[] = "shell> ";
char input_buffer[256] = "\0";
void * command_handlers[256];

extern void * heap_start_vaddr;

/* ------------------------------------------------------------------------- */

void shell_init() {
    vga_scroll_up(1, TOP_LINE);
    keyboard_register_handler((void*)shell_keyboard_handler);
    shell_print_prompt();
    while(true) { __asm__("nop"); }
}

/* ------------------------------------------------------------------------- */

void shell_print_prompt() {
    vga_overwrite_line("shell> ", VGA_COLOUR_WHITE, VGA_COLOUR_BLACK, 24);
    vga_set_cursor(7, 24);
}

/* ------------------------------------------------------------------------- */

void shell_delete_from_input_buffer() {
    input_buffer[strlen(input_buffer)] = '\0';
    input_buffer[strlen(input_buffer)-1] = '\0';
}

/* ------------------------------------------------------------------------- */

void shell_clear_input_buffer() {
    memset((void*)&input_buffer, 0, sizeof(input_buffer));
}

/* ------------------------------------------------------------------------- */

void shell_keyboard_handler(uint8_t key) {
    // If the key is Enter, submit the command and scroll existing lines up
    if(key == KEY_ENTER) {
        vga_scroll_up(1, TOP_LINE);
        vga_state.cursor_x = 0;
        shell_process_command(input_buffer);
        shell_clear_input_buffer();
        shell_print_prompt();
        return;
    }

    if(key == KEY_BACKSPACE) {
        shell_delete_from_input_buffer();
        vga_delete_char_at_cursor();
        return;
    }

    if(key != 0x00) {
        *(&key + 1) = '\0';
        strcat(input_buffer, (char*)&key);
        vga_print_char_at_cursor(key);
    }
}

/* ------------------------------------------------------------------------- */

extern uint32_t t1_counter;

void apple() {
    printk(LOG_INFO, "APPLE initialised\n");

    char * test1 = (char*)0x300100;
    int magic = 0;

    while(true) {
        magic++;
        if(magic == 20000000) {
            klog("Message from kernel task APPLE!\n");
            *test1 = 'A';
            klog("test1 (0x%x) is: %s\n", test1, test1);
            magic = 0;
        }
    }
}

void banana() {
    printk(LOG_INFO, "BANANA initialised\n");
    while(true) { }
}

#include <rotary/test/ktest.h>

struct task * ktask1 = NULL;

void shell_process_command(char * command) {

#include <rotary/fs/pseudo/tree.h>
    if(strcmp(command, "pseudo") != 0) {

        struct pseudo_node dir1;
        dir1.name = "example_dir";
        dir1.mode = PSEUDO_DIR;
        dir1.children.prev = NULL;
        dir1.children.next = NULL;

        struct pseudo_node dir1_child1;
        dir1_child1.name = "file1";
        dir1_child1.mode = PSEUDO_DIR;
        dir1_child1.prev = NULL;
        dir1_child1.next = NULL;

        dir1.children



        
    }



    if(strcmp(command, "kt") != 0) {
        ktask1 = task_create("ktask_test1", TASK_KERNEL, &apple,
                                       TASK_PRIORITY_MIN,
                                       TASK_STATE_WAITING);
        struct vm_map * map = vm_map_new();
        map->start_addr = (void*)0x300000;
        map->end_addr   = (void*)0x400000;
        vm_space_add_map(ktask1->vm_space, map);
        return;
    }

#include <rotary/fs/vfs/fs_type.h>
    if(strcmp(command, "fstype")) {
        file_system_type_print_debug();

        struct file_system_type fs_type = {
            .name = "test123",
        };
        file_system_type_register(&fs_type);

        struct file_system_type * test = file_system_type_get("test123");
        klog("found at: 0x%x\n", test);

        file_system_type_print_debug();

        file_system_type_unregister(&fs_type);

        file_system_type_print_debug();

        test = file_system_type_get("test123");
        klog("post unreg at: 0x%x\n", test);

    }
    
    if(strcmp(command, "vm-test")) {
        ktest_run_module("vm");
    }

    if(strcmp(command, "palloc-test")) {
        ktest_run_module("palloc");
    }


    if(strcmp(command, "run-tests")) {
        ktest_run_all();
    }

    if(strcmp(command, "dump-kmalloc")) {
        kmalloc_print_debug();
    }

    if(strcmp(command, "ut") != 0) {
        struct task * task = task_create("utask_test1", TASK_USERMODE, (void*)0x400000,
                    TASK_PRIORITY_MIN, TASK_STATE_WAITING);
        struct vm_map * map = vm_map_new();
        map->start_addr = (void*)0x400000;
        map->end_addr   = (void*)0x600000;
        vm_space_add_map(task->vm_space, map);
        return;
    }

    if(strcmp(command, "clear") != 0 || strcmp(command, "cls") != 0) {
        for(uint32_t i = 4; i < VGA_HEIGHT; i++) {
            vga_clear_line(i);
        }
        return;
    }

    if(strcmp(command, "build") != 0) {
        vga_printf("Kernel built: %s %s", __DATE__, __TIME__);
        return;
    }

    if(strcmp(command, "tasks") != 0) {
        task_print();
        return;
    }

    if(strcmp(command, "slab") != 0) {
        kmalloc_print_debug();
    }

    if(strcmp(command, "buddy") != 0) {
        buddy_print_debug();
    }

    if(strcmp(command, "mem") != 0) {
        //paging_print_debug();
        klog("\n");
        buddy_print_debug();
        klog("\n");
        bootmem_print_debug();
        klog("\n");
    }

    vga_printf("Unknown command: %s", command);
}

/* ------------------------------------------------------------------------- */

void shell_register_handler(char * command, void * handler) {
}

/* ------------------------------------------------------------------------- */
