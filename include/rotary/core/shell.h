/* ========================================================================= */
/* Kernel Debugging Shell                                                    */
/* ========================================================================= */
#pragma once

#include <rotary/core.h>
#include <rotary/drivers/input/keyboard/keyboard.h>
#include <rotary/sched/task.h>
#include <rotary/mm/ptable.h>
#include <rotary/mm/palloc.h>

#include <arch/vga.h>

/* ========================================================================= */

void shell_init();
void shell_keyboard_handler(uint8_t key);
void shell_print_prompt();
void shell_register_handler(char * command, void * handler);
void shell_process_command(char * command);

/* ========================================================================= */
