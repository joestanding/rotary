/* ========================================================================= */
/* Rotary OS                                                                 */
/* ========================================================================= */

#include <rotary/core/main.h>

void kernel_main(uint32_t arg1, uint32_t arg2) {

    arch_init(arg1, arg2);

    while(true) { }
}
