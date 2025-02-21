/*
 * include/rotary/logging.h
 * Debug Logging
 */

#ifndef INC_LOGGING_H
#define INC_LOGGING_H

#include <rotary/core.h>
#include <rotary/vga.h>
#include <rotary/drivers/tty/serial.h>
#include <rotary/sync.h>

/* ------------------------------------------------------------------------- */

#define LOG_TRACE 0
#define LOG_DEBUG 1
#define LOG_INFO  2

#define printk(log_level, fmt_str, ...) \
    { \
        char tmp[512]; \
        memset(&tmp, 0x00, 512); \
        sprintf(tmp, fmt_str, ##__VA_ARGS__); \
        _printk(log_level, __FILE__, tmp); \
    }

#define klog(fmt_str, ...) \
    { \
        char tmp[512]; \
        memset(&tmp, 0x00, 512); \
        sprintf(tmp, fmt_str, ##__VA_ARGS__); \
        _printk(LOG_DEBUG, __FILE__, tmp); \
    }

/* ------------------------------------------------------------------------- */

void _printk(uint8_t level, char * tag, char * message);

/* ------------------------------------------------------------------------- */

#endif
