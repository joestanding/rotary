/*
 * include/rotary/vga.h
 * Platform-independent VGA text mode
 */

#ifndef INC_VGA_H
#define INC_VGA_H

#include <rotary/core.h>
#include <rotary/string.h>
#include <rotary/logging.h>
#include <arch/vga.h>

/* ------------------------------------------------------------------------- */

#define VGA_COLOUR_BLACK        0
#define VGA_COLOUR_BLUE         1
#define VGA_COLOUR_GREEN        2
#define VGA_COLOUR_CYAN         3
#define VGA_COLOUR_RED          4
#define VGA_COLOUR_MAGENTA      5
#define VGA_COLOUR_BROWN        6
#define VGA_COLOUR_WHITE        7

/* ------------------------------------------------------------------------- */

#define vga_printf(fmt_str, ...) \
    { \
        char tmp[512]; \
        memset(&tmp, 0, sizeof(tmp)); \
        sprintf(tmp, fmt_str, ##__VA_ARGS__); \
        vga_write(tmp); \
    }

#define vga_printf_pos(x, y, fmt_str, ...) \
    { \
        char tmp[512]; \
        memset(&tmp, 0, sizeof(tmp)); \
        sprintf(tmp, fmt_str, ##__VA_ARGS__); \
        vga_set_cursor(x, y); \
        vga_write(tmp); \
    }

/* ------------------------------------------------------------------------- */

typedef struct {
    uint32_t cursor_x;
    uint32_t cursor_y;
    uint8_t  enabled;
} vga_state_t;

/* ------------------------------------------------------------------------- */

extern vga_state_t vga_state;

/* ------------------------------------------------------------------------- */

int32_t arch_vga_init();
void vga_write(char * string);
void vga_set_cursor(uint32_t x, uint32_t y);
void vga_print_char_at_cursor(uint8_t tchar);
void vga_print_string_at_cursor(char * string);
void vga_delete_char_at_cursor();
void vga_print_line(char * string);
void vga_scroll_up(int line_count, int top_line);
void vga_clear_screen();
void vga_clear_line(int line);
void vga_overwrite_line(char * string, uint8_t fg_colour, uint8_t bg_colour,
                        int y);
void vga_set_character(uint8_t character, uint8_t fg_colour, uint8_t bg_colour,
                       uint32_t x, uint32_t y);

/* ------------------------------------------------------------------------- */

#endif
