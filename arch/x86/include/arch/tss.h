/*
 * arch/x86/include/arch/tss.h
 * Task State Segment (TSS)
 */

#ifndef INC_ARCH_TSS_H
#define INC_ARCH_TSS_H

#include <rotary/core.h>
#include <rotary/logging.h>

/* ------------------------------------------------------------------------- */

struct tss {
    uint16_t previous_task, __previous_task_reserved;
    uint32_t esp0;
    uint16_t ss0, __ss0_reserved;
    uint32_t esp1;
    uint16_t ss1, __ss1_reserved;
    uint32_t esp2;
    uint16_t ss2, __ss2_reserved;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint16_t es, __es_reserved;
    uint16_t cs, __cs_reserved;
    uint16_t ss, __ss_reserved;
    uint16_t ds, __ds_reserved;
    uint16_t fs, __fs_reserved;
    uint16_t gs, __gs_reserved;
    uint16_t ldt_selector, __ldt_sel_reserved;
    uint16_t debug_flag, io_map;
} __attribute__((packed));

/* ------------------------------------------------------------------------- */

int32_t tss_init();
int32_t tss_set_esp0(void * esp0);
int32_t tss_set_ss0(uint32_t ss0);
struct tss * tss_get();

extern void tss_flush();

/* ------------------------------------------------------------------------- */

#endif
