/*
 * include/rotary/test/ktest.h
 * Kernel Unit Testing
 */

#ifndef INC_TEST_KTEST_H
#define INC_TEST_KTEST_H

#include <rotary/core.h>
#include <rotary/debug.h>
#include <rotary/util/math.h>
#include <rotary/logging.h>

/* ------------------------------------------------------------------------- */

typedef struct ktest_unit {
    char * name;
    uint32_t pass_count;
    uint32_t fail_count;
    void (*test_func)(struct ktest_unit *);
} ktest_unit_t;

typedef struct ktest_module {
    char * name;

    int32_t (*module_pre)(struct ktest_module *);
    int32_t (*module_post)(struct ktest_module *);
    int32_t (*test_pre)(struct ktest_module *);
    int32_t (*test_post)(struct ktest_module *);

    ktest_unit_t * tests;
    uint32_t test_count;
    uint32_t fail_count;
} ktest_module_t;

/* ------------------------------------------------------------------------- */

void assert_equal_int(ktest_unit_t * ktest, int expected, int actual,
                      char * file, int line);
void assert_equal_uint(ktest_unit_t * ktest, unsigned int expected,
                       unsigned int actual, char * file, int line);
void assert_equal_ptr(ktest_unit_t * ktest, void * expected, void * actual,
                      char * file, int line);
void assert_equal_str(ktest_unit_t * ktest, char * expected,
                      char * actual, char * file, int line);

void assert_not_equal_int(ktest_unit_t * ktest, int expected, int actual,
                          char * file, int line);
void assert_not_equal_uint(ktest_unit_t * ktest, unsigned int expected,
                       unsigned int actual, char * file, int line);
void assert_not_equal_ptr(ktest_unit_t * ktest, void * expected, void * actual,
                          char * file, int line);
void assert_not_equal_str(ktest_unit_t * ktest, char * expected,
                          char * actual, char * file, int line);

void _assert_filled(ktest_unit_t * ktest, void * buffer, uint32_t size,
                    unsigned char value, char * file, int line);

void _assert_bit_set(ktest_unit_t * ktest, unsigned int buffer,
                     unsigned int bit, char * file, int line);

void ktest_run_all();
void ktest_run_module(char * module_name);
ktest_module_t * ktest_get_module(char * module_name);
void ktest_list_modules();

/* ------------------------------------------------------------------------- */

#define KTEST_UNIT(n, f) \
    { \
        .name = (n), \
        .fail_count = 0, \
        .test_func = (f), \
    }

/* We need test modules to register themselves dynamically without something 
 * calling a registration function manually during the init process. This macro
 * uses __attribute__((section(".ktest"))) to ensure that these structures are
 * located within the .ktest section of the final ELF file. This allows the
 * ktest_module list to be easily iterated through at runtime, and ktest files
 * can be added without need to add each to some sort of registry manually. */
#define KTEST_MODULE_DEFINE(n, tsts, mpre, mpost, tpre, tpost) \
    static __attribute__((used)) \
    __attribute__((section(".ktest"))) \
    ktest_module_t TP(ktest_module, __COUNTER__) = \
    { \
        .name        = (n), \
        .tests       = (tsts), \
        .module_pre  = (mpre), \
        .module_post = (mpost), \
        .test_pre    = (tpre), \
        .test_post   = (tpost), \
        .test_count  = ARRAY_SIZE((tsts)) \
    }

#define assert(condition) \
    do { \
        if (!(condition)) { \
            klog("[%s] ASSERTION FAILED: %s (file %s, line %d)\n", \
                 ktest->name, #condition, __FILE__, __LINE__); \
            ktest->fail_count++; \
        } else { \
            ktest->pass_count++; \
        } \
    } while (0)

#define assert_equal(expected, actual) \
    do { \
        _Generic((expected), \
            int: assert_equal_int, \
            unsigned int: assert_equal_uint, \
            char*: assert_equal_str, \
            default: assert_equal_ptr \
        )(ktest, expected, actual, __FILE__, __LINE__); \
    } while (0)

#define assert_not_equal(expected, actual) \
    do { \
        _Generic((expected), \
            int: assert_not_equal_int, \
            unsigned int: assert_not_equal_uint, \
            char*: assert_not_equal_str, \
            default: assert_not_equal_ptr \
        )(ktest, expected, actual, __FILE__, __LINE__); \
    } while (0)

#define assert_clear(buffer, size) \
    do { \
        _assert_filled(ktest, buffer, size, 0, __FILE__, __LINE__); \
    } while (0)

#define assert_filled(buffer, size, value) \
    do { \
        _assert_filled(ktest, buffer, size, value, __FILE__, __LINE__); \
    } while (0)

#define assert_bit_set(buffer, bit) \
    do { \
        _assert_bit_set(ktest, buffer, bit, __FILE__, __LINE__); \
    } while (0)

/* ------------------------------------------------------------------------- */

#endif
