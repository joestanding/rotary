/*
 * arch/x86/include/arch/msr.h
 * Model Specific Registers
 */

#ifndef INC_ARCH_MSR_H
#define INC_ARCH_MSR_H

/* ------------------------------------------------------------------------- */

static inline void msr_write(uint32_t reg, uint64_t value) {
    uint32_t low  = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;

    asm volatile("wrmsr"
                 :
                 : "c"(reg), "a"(low), "d"(high)
                 : "memory"
    );
}

/* ------------------------------------------------------------------------- */

#endif
