/*
 * kernel/keyboard.c
 * PS/2 Keyboard
 *
 * Handles I/O to and from a PS/2 keyboard.
 */

#include <rotary/drivers/input/keyboard/keyboard.h>

// TODO: Replace with linked-list approach
void * keyboard_event_handlers[MAX_INTERRUPT_HANDLERS];

/* ------------------------------------------------------------------------- */

/**
 * keyboard_init() - Perform keyboard initialisation.
 *
 * Return: E_SUCCESS
 */
int32_t keyboard_init() {
    arch_keyboard_init();
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * keyboard_register_handler() - Register a handler for keyboard activity.
 * @handler_addr: Pointer to the handler function.
 *
 * Return: E_SUCCESS;
 */
int32_t keyboard_register_handler(void * handler_addr) {
    for(uint32_t i = 0; i < MAX_INTERRUPT_HANDLERS; i++ ) {
        if(keyboard_event_handlers[i] == NULL) {
            keyboard_event_handlers[i] = handler_addr;
            return E_SUCCESS;
        }
    }

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * keyboard_handle_input() - Handle keyboard input.
 * @keypress: The key that has been pressed.
 *
 * Iterate through the list of registered keyboard event handlers and pass
 * the keypress to each of them.
 *
 * Return: E_SUCCESS
 */
int32_t keyboard_handle_input(uint8_t keypress) {
    uint8_t key = keyboard_translate_scan_code(keypress);
    for(uint32_t i = 0; i < MAX_INTERRUPT_HANDLERS; i++) {
        if(keyboard_event_handlers[i] != NULL) {
            keyboard_handler * kh =
                (keyboard_handler*)keyboard_event_handlers[i];
            kh(key);
        }
    }

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * keyboard_translate_scan_code() - Translate a scan code to ASCII.
 * @scan_code: The scan code to be translated.
 *
 * Return: The ASCII value of the scan code.
 */
char keyboard_translate_scan_code(uint8_t scan_code) {
    switch(scan_code) {
        case 0x01:
            return 0x00;
        case 0x02:
            return 0x02;
        case 0x03:
            return '2';
        case 0x04:
            return '3';
        case 0x05:
            return '4';
        case 0x06:
            return '5';
        case 0x07:
            return '6';
        case 0x08:
            return '7';
        case 0x09:
            return '8';
        case 0x0A:
            return '9';
        case 0x0B:
            return '0';
        case 0x0C:
            return '-';
        case 0x0D:
            return '=';
        case 0x0E:
            return 0x0E;
        case 0x0F:
            return 0x0F;
        case 0x10:
            return 'q';
        case 0x11:
            return 'w';
        case 0x12:
            return 'e';
        case 0x13:
            return 'r';
        case 0x14:
            return 't';
        case 0x15:
            return 'y';
        case 0x16:
            return 'u';
        case 0x17:
            return 'i';
        case 0x18:
            return 'o';
        case 0x19:
            return 'p';
        case 0x1A:
            return '[';
        case 0x1B:
            return ']';
        case 0x1C:
            return 0x1C; // Enter
        case 0x1D:
            return 0x1D; // Left Control
        case 0x1E:
            return 'a';
        case 0x1F:
            return 's';
        case 0x20:
            return 'd';
        case 0x21:
            return 'f';
        case 0x22:
            return 'g';
        case 0x23:
            return 'h';
        case 0x24:
            return 'j';
        case 0x25:
            return 'k';
        case 0x26:
            return 'l';
        case 0x27:
            return ';';
        case 0x28:
            return '\'';
        case 0x29:
            return '`';
        case 0x2A:
            return 0x2A; // Left Shift
        case 0x2B:
            return '\\';
        case 0x2C:
            return 'z';
        case 0x2D:
            return 'x';
        case 0x2E:
            return 'c';
        case 0x2F:
            return 'v';
        case 0x30:
            return 'b';
        case 0x31:
            return 'n';
        case 0x32:
            return 'm';
        case 0x33:
            return ',';
        case 0x34:
            return '.';
        case 0x35:
            return '/';
        case 0x36:
            return 0x36; // Right Shift
        case 0x37:
            return '*';
        case 0x38:
            return 0x38; // Left Alt
        case 0x39:
            return ' ';
        case 0x3A:
            return 0x3A; // Caps Lock
        case 0x3B:
            return 0x3B; // F1
        case 0x3C:
            return 0x3C; // F2
        default:
            return 0x00;
    }
}

/* ------------------------------------------------------------------------- */
