#include <rotary/util/math.h>

/* ------------------------------------------------------------------------- */

uint32_t log2(uint32_t n) {
    uint32_t result = 0;
    while (n >>= 1) {
        result++;
    }
    return result;
}

/* ------------------------------------------------------------------------- */
