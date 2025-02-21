/*
 * include/rotary/syscall.h
 * Syscalls
 */

#ifndef INC_SYSCALL_H
#define INC_SYSCALL_H

#include <rotary/core.h>
#include <rotary/logging.h>
#include <rotary/debug.h>
#include <rotary/sched/task.h>
#include <arch/syscall.h>

/* ------------------------------------------------------------------------- */

#define SYSCALL_READ        0x00
#define SYSCALL_WRITE       0x01
#define SYSCALL_OPEN        0x02
#define SYSCALL_CLOSE       0x03
#define SYSCALL_EXIT        0x04

/* ------------------------------------------------------------------------- */

void syscall_write(int descriptor_id, void * src_buffer, size_t size);

/* ------------------------------------------------------------------------- */

#endif
