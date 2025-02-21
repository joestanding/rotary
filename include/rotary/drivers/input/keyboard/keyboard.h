/*
 * include/rotary/drivers/keyboard.h
 */

#ifndef INC_DRIVERS_KEYBOARD_H
#define INC_DRIVERS_KEYBOARD_H

#include <rotary/core.h>
#include <arch/keyboard.h>

/* ------------------------------------------------------------------------- */

#define KEY_ENTER       0x1C
#define KEY_BACKSPACE   0x0E

typedef void keyboard_handler(uint8_t key);

/* ------------------------------------------------------------------------- */

int32_t keyboard_init();
int32_t keyboard_register_handler(void * handler_addr);
int32_t keyboard_handle_input(uint8_t keypress);
char    keyboard_translate_scan_code(uint8_t scan_code);

/* ------------------------------------------------------------------------- */

int32_t arch_keyboard_init();

/* ------------------------------------------------------------------------- */

#endif
