/*
 * include/rotary/core/interrupts.h
 * Architecture-independent Interrupt Handlers
 */

#pragma once

#include <rotary/core.h>
#include <rotary/debug.h>
#include <arch/interrupts.h>

void int_handle_page_fault(void * fault_addr);
