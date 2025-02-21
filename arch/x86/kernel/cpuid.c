/*
 * cpuid.c
 * CPUID Interface
 *
 * Retrieve processor feature information using the x86 CPUID instruction.
 * CPUID information can be used to identify features supported by the CPU,
 * such as support for large pages or extension instruction sets.
 */

#include <arch/cpuid.h>

/* ------------------------------------------------------------------------- */

uint32_t leaf0_eax, leaf0_ebx, leaf0_ecx, leaf0_edx;
uint32_t leaf1_eax, leaf1_ebx, leaf1_ecx, leaf1_edx;
uint32_t leaf2_eax, leaf2_ebx, leaf2_ecx, leaf2_edx;
uint32_t leaf3_eax, leaf3_ebx, leaf3_ecx, leaf3_edx;

/* ------------------------------------------------------------------------- */

/**
 * cpuid_init() - Retrieve CPU information using the CPUID instruction.
 *
 * Retrieves all available CPUID information by calling the GCC intrinsic
 * __get_cpuid(). Retrieved information can then be returned immediately
 * to kernel components later on by calling the relevant functions.
 *
 * Return: E_SUCCESS
 */
int32_t cpuid_init() {
    klog("cpuid_init(): Retrieving CPUID values..\n");
    __get_cpuid(0, &leaf0_eax, &leaf0_ebx, &leaf0_ecx, &leaf0_edx);
    __get_cpuid(1, &leaf1_eax, &leaf1_ebx, &leaf1_ecx, &leaf1_edx);
    __get_cpuid(2, &leaf2_eax, &leaf2_ebx, &leaf2_ecx, &leaf2_edx);
    __get_cpuid(3, &leaf3_eax, &leaf3_ebx, &leaf3_ecx, &leaf3_edx);
    char cpu_name[24];
    cpuid_get_cpu_name(cpu_name);
    klog("CPU: %s\n", cpu_name);

    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * cpuid_get_cpu_name() - Retrieves the CPU vendor ID name.
 * @dest_buf: A pointer to the buffer in which to store the CPU name.
 *            Must be at least 13 bytes in size.
 *
 * Returns the CPU vendor ID string. The vendor ID string is kept in three
 * separate registers and must be re-assembled from them. Vendor ID strings
 * are always 12 characters long, for example "AuthenticAMD". The vendor ID
 * string does not provide the full model name of the processor.
 *
 * Return: E_SUCCESS
 */
int32_t cpuid_get_cpu_name(char * dest_buf) {
    uint32_t eax, ebx, ecx, edx;
    __get_cpuid(0, &eax, &ebx, &ecx, &edx);

    sprintf(dest_buf, "%c%c%c%c%c%c%c%c%c%c%c%c\0",
            (ebx & 0xFF), (ebx >> 8 & 0xFF), (ebx >> 16 & 0xFF),
            (ebx >> 24 & 0xFF), (edx & 0xFF), (edx >> 8 & 0xFF),
            (edx >> 16 & 0xFF), (edx >> 24 & 0xFF), (ecx & 0xFF),
            (ecx >> 8 & 0xFF), (ecx >> 16 & 0xFF), (ecx >> 24 & 0xFF));
    return E_SUCCESS;
}

/* ------------------------------------------------------------------------- */

/**
 * cpuid_check_pse() - Returns whether Page Size Extension is supported.
 *
 * Return: 1 if Page Size Extension is supported, 0 if it isn't.
 */
int32_t cpuid_check_pse() {
    return (leaf1_edx & CPUID_FEAT_EDX_PSE) != 0 ? 1 : 0;
}

/* ------------------------------------------------------------------------- */

/**
 * cpuid_check_pge() - Returns whether Page Global Enable is supported.
 *
 * Return: 1 if Page Global Enable is supported, 0 if it isn't.
 */
int32_t cpuid_check_pge() {
    return (leaf1_edx & CPUID_FEAT_EDX_PGE) != 0 ? 1 : 0;
}

/* ------------------------------------------------------------------------- */

/**
 * cpuid_check_apic() - Returns whether the Intel APIC is present.
 *
 * Return: 1 if the APIC is present, 0 if it isn't.
 */
int32_t cpuid_check_apic() {
    return (leaf1_edx & CPUID_FEAT_EDX_APIC) != 0 ? 1 : 0;
}

/* ------------------------------------------------------------------------- */

/**
 * cpuid_check_x2apic() - Returns whether the Intel x2APIC is present.
 *
 * Return: 1 if the x2APIC is present, 0 if it isn't.
 */
int32_t cpuid_check_x2apic() {
    return (leaf1_edx & CPUID_FEAT_ECX_X2APIC) != 0 ? 1 : 0;
}

/* ------------------------------------------------------------------------- */

/**
 * x86_paging_pse_enabled() - Check whether Page Size Extensions are enabled.
 *
 * First checks whether Page Size Extensions (PSE) are available on the CPU by
 * checking the CPUID data - if available, identifies whether PSE is enabled by
 * checking the relevant bit in the CR4 control register.
 *
 * Return: 0 if disabled, 1 if enabled.
 */
int32_t x86_paging_pse_enabled() {
    if(!cpuid_check_pse())
        return 0;

    uint32_t cr4;
    __asm__ volatile ("mov %%cr4, %0" : "=r" (cr4));
    return (cr4 & 0x10) ? 1 : 0;
}

/* ------------------------------------------------------------------------- */

/**
 * x86_paging_pge_enabled() - Check whether Page Global Enable is enabled.
 *
 * First checks whether Page Global Enable is available on the CPU by
 * checking the CPUID data - if available, identifies whether PGE is enabled by
 * checking the relevant bit in the CR4 control register.
 *
 * Return: 0 if disabled, 1 if enabled.
 */

int32_t x86_paging_pge_enabled() {
    if(!cpuid_check_pge())
        return 0;

    uint32_t cr4;
    __asm__ volatile ("mov %%cr4, %0" : "=r" (cr4));
    return (cr4 & 0x80) ? 1 : 0;
}

/* ------------------------------------------------------------------------- */
