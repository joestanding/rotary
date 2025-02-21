/*
 * include/rotary/drivers/tty/tty.h
 * Teletype Terminal Emulation
 */

#ifndef INC_DRIVERS_TTY_H
#define INC_DRIVERS_TTY_H

#include <rotary/core.h>
#include <rotary/list.h>
#include <rotary/mm/kmalloc.h>
#include <rotary/logging.h>
#include <rotary/debug.h>

/* ------------------------------------------------------------------------- */

#define TTY_ID_DEFAULT 1
#define TTY_ID_INVALID 9999

#define TTY_DEFAULT_BUFFER_SIZE 4096
#define TTY_MAX_COUNT 4

/* ------------------------------------------------------------------------- */

struct tty {
    uint32_t    id;
    char *      input_buffer;
    uint32_t    input_buffer_bytes;
    char *      output_buffer;
    uint32_t    output_buffer_bytes;
    uint32_t    buffer_size;
    list_node_t list_entry;
};

/* ------------------------------------------------------------------------- */

int32_t tty_init();

struct tty * tty_create(uint32_t id, uint32_t type, uint32_t buffer_size);
int32_t tty_destroy(struct tty * target);

int32_t tty_write(struct tty * target, char * buffer, uint32_t count);

int32_t tty_flush(struct tty * target);
struct tty * tty_get_default();

/* ------------------------------------------------------------------------- */

#endif
