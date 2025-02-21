/*
 * include/rotary/core.h
 * Core Definitions, Types and Macros
 *
 * Provides core definitions, types and macros that will be used throughout
 * the codebase regardless of architecture.
 */

#ifndef INC_CORE_H
#define INC_CORE_H

#include <stdint.h>
#include <stddef.h>

/* ------------------------------------------------------------------------- */

#define false 0
#define true 1

#define SIZE_1K 1024
#define SIZE_1M (1024 * 1024)
#define SIZE_1G (1024 * 1024 * 1024)
#define SIZE_4K 4096
#define SIZE_4M (4096 * 1024)

/* ------------------------------------------------------------------------- */

#define E_SUCCESS 0
#define E_ERROR   -1

#define SUCCESS(value) ((value) >= 0)
#define PTR_SUCCESS(ptr) ((ptr) != (void*)-1)

/* ------------------------------------------------------------------------- */

#define container_of(ptr, type, member) ({ \
        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type, member) );})

#define SET_BIT(flags, bit) ((flags) |= (bit))
#define CLEAR_BIT(flags, bit) ((flags) &= ~(bit))
#define TEST_BIT(flags, bit) ((flags) & (bit))

#define TP(x, y) TP_IMPL(x, y)
#define TP_IMPL(x, y) x ## y

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(*array))

/* ------------------------------------------------------------------------- */

typedef uint32_t flags_t;

/* ------------------------------------------------------------------------- */

#endif
