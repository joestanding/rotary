/*
 * kernel/drivers/tty/tty.c
 * Teletypewriter
 *
 * TTY implementation for pseudo-terminals and serial output.
 */

#include <rotary/drivers/tty/tty.h>

struct tty tty_list_head;

/* ------------------------------------------------------------------------- */

/**
 * tty_init() - Initialise default TTYs.
 *
 * Return: E_SUCCESS
 */
int32_t tty_init() {
    klog("Initialising default TTYs..\n");

    // Initialise TTY list head and mark it as an invalid TTY
    llist_init(&tty_list_head.list_entry);
    tty_list_head.id = TTY_ID_INVALID;

    // Create our default TTY
    struct tty * default_tty = tty_create(TTY_ID_DEFAULT, 0, TTY_DEFAULT_BUFFER_SIZE);
    if(!SUCCESS(default_tty)) {
        klog("Failed to create initial TTY!\n");
        return E_ERROR;
    }

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * tty_create() - Create a new TTY.
 * @id:          The ID of the new TTY.
 * @type:        The type of the TTY.
 * @buffer_size: The default TTY I/O buffer size.
 *
 * Return: A pointer to the newly created TTY object.
 */
struct tty * tty_create(uint32_t id, uint32_t type, uint32_t buffer_size) {
    printk(LOG_TRACE, "tty_create(id: %d, type: %d, bufsize: %d)\n",
            id, type, buffer_size);

    // Allocate and clear memory for the TTY
    struct tty * new_tty = (struct tty*)kmalloc(sizeof(struct tty));
    if(!SUCCESS(new_tty)) {
        klog("Failed to allocate memory for TTY object!\n");
        return (struct tty*)E_ERROR;
    }
    memset(new_tty, 0, sizeof(struct tty));

    // Set TTY ID
    new_tty->id = id;

    // Allocate input and output buffers
    new_tty->buffer_size   = buffer_size;
    new_tty->input_buffer  = kmalloc(new_tty->buffer_size);
    new_tty->output_buffer = kmalloc(new_tty->buffer_size);
    if(!SUCCESS(new_tty->input_buffer)|| !SUCCESS(new_tty->output_buffer)) {
        klog("Failed to allocate I/O buffers for TTY id %d!\n", id);
        return (struct tty*)E_ERROR;
    }
    memset(new_tty->input_buffer, 0, new_tty->buffer_size);
    memset(new_tty->output_buffer, 0, new_tty->buffer_size);

    // Add our TTY to the global TTY list
    llist_add_after(&tty_list_head.list_entry, &new_tty->list_entry);

    return new_tty;
}

/* ------------------------------------------------------------------------- */

/**
 * tty_destroy() - Destroy a TTY object.
 * @target: A pointer to the TTY object to be destroyed.
 *
 * Return: E_SUCCESS
 */
int32_t tty_destroy(struct tty * target) {
    // Remove the TTY from the global TTY list
    llist_delete_node(&target->list_entry);
    // De-allocate the memory for the IO buffers
    kfree(target->input_buffer);
    kfree(target->output_buffer);
    // De-allocate the memory for the TTY object
    kfree(target);
    // Clear the memory of the TTY object
    memset(target, 0, sizeof(struct tty));

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * tty_write() - Write to a TTY output buffer.
 * @target: The TTY object to be written to.
 * @buffer: A pointer to a buffer containing ASCII data to be written.
 * @count:  The amount of bytes to be written to the TTY output buffer.
 *
 * Return: E_SUCCESS
 */
int32_t tty_write(struct tty * target, char * buffer, uint32_t count) {
    klog("tty_write(tty: 0x%x | buffer: 0x%x | count: %d)\n",
            target,
            buffer,
            count);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * tty_flush() - Flush a TTY's input and output buffers.
 *
 * Return: E_SUCCESS
 */
int32_t tty_flush(struct tty * target) {
    // TODO: Implement
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * tty_get_default() - Retrieve a pointer to the default TTY.
 *
 * Return: A pointer to the default TTY object.
 */
struct tty * tty_get_default() {
    // TODO: Implement
    return NULL;
}

/* ------------------------------------------------------------------------- */
