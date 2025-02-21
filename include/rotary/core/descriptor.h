/* ========================================================================= */
/* Descriptors                                                               */
/* ========================================================================= */

#pragma once

#include <rotary/core.h>
#include <rotary/list.h>

/* ========================================================================= */

#define DESCRIPTOR_TYPE_NONE        0x00
#define DESCRIPTOR_TYPE_FILE        0x01
#define DESCRIPTOR_TYPE_SOCKET      0x02
// TODO: Remove this once filesystem support is implemented,
// then output from processes can go to a file, then to the TTY
#define DESCRIPTOR_TYPE_TTY         0x99
#define DESCRIPTOR_TYPE_INVALID     9999

#define DESCRIPTOR_ACCESS_READ      0x01
#define DESCRIPTOR_ACCESS_WRITE     0x02
#define DESCRIPTOR_ACCESS_RW        0x03

/* ========================================================================= */

// TODO: Consider synchronisation primitives
typedef struct {
    uint32_t id;          // Numeric identifier for the descriptor
    uint32_t type;        // Type (file, socket, etc.)
    uint32_t access;      // Read, Write or Read/Write
    void *   resource;    // Ptr to resource object (file, socket, etc.)
    list_node_t list_entry;
} descriptor;

/* ========================================================================= */
