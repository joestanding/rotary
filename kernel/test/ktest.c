/*
 * kernel/test/ktest.c
 * Kernel Unit Testing
 */

#include <rotary/test/ktest.h>

/* ------------------------------------------------------------------------- */

extern ktest_module_t __start_ktest[];
extern ktest_module_t __stop_ktest[];

/* ------------------------------------------------------------------------- */

void assert_equal_int(ktest_unit_t * ktest, int expected, int actual,
                      char * file, int line) {
    if (expected != actual) {
        klog("[%s] int ASSERTION FAILED! exp %d != act %d (file %s line %d)\n",
             ktest->name, expected, actual, file, line);
        ktest->fail_count++;
    } else {
        ktest->pass_count++;
    }
}

/* ------------------------------------------------------------------------- */

void assert_equal_uint(ktest_unit_t * ktest, unsigned int expected,
                       unsigned int actual, char * file, int line) {
    if (expected != actual) {
        klog("[%s] uint ASSERTION FAILED! %u != %u (in file %s, line %d)\n",
             ktest->name, expected, actual, file, line);
        ktest->fail_count++;
    } else {
        ktest->pass_count++;
    }
}

/* ------------------------------------------------------------------------- */

void assert_equal_str(ktest_unit_t * ktest, char *expected, char *actual,
                      char * file, int line) {
    if (strcmp(expected, actual) != 0) {
        klog("Assertion failed: \"%s\" != \"%s\" (in file %s, line %d)\n",
             expected, actual, file, line);
        ktest->fail_count++;
    } else {
        ktest->pass_count++;
    }
}

/* ------------------------------------------------------------------------- */

void assert_equal_ptr(ktest_unit_t * ktest, void *expected, void *actual,
                      char * file, int line) {
    if (expected != actual) {
        klog("[%s] ptr ASSERTION FAILED! %p != %p (in file %s, line %d)\n",
             ktest->name, expected, actual, file, line);
        ktest->fail_count++;
    } else {
        ktest->pass_count++;
    }
}

/* ------------------------------------------------------------------------- */

void assert_not_equal_int(ktest_unit_t * ktest, int expected, int actual,
                          char * file, int line) {
    if (expected == actual) {
        klog("[%s] ASSERTION FAILED! Expected %d != %d (in file %s, line %d)\n",
             ktest->name, expected, actual, file, line);
        ktest->fail_count++;
    } else {
        ktest->pass_count++;
    }
}

/* ------------------------------------------------------------------------- */

void assert_not_equal_uint(ktest_unit_t * ktest, unsigned int expected,
                           unsigned int actual, char * file, int line) {
    if (expected == actual) {
        klog("[%s] ASSERTION FAILED! Expected %u != %u (in file %s, line %d)\n",
             ktest->name, expected, actual, file, line);
        ktest->fail_count++;
    } else {
        ktest->pass_count++;
    }
}

/* ------------------------------------------------------------------------- */

void assert_not_equal_ptr(ktest_unit_t * ktest, void *expected, void *actual,
                          char * file, int line) {
    if (expected == actual) {
        klog("[%s] ASSERTION FAILED! Expected pointers to differ: %p == %p (in file %s, line %d)\n",
             ktest->name, expected, actual, file, line);
        ktest->fail_count++;
    } else {
        ktest->pass_count++;
    }
}

/* ------------------------------------------------------------------------- */

void assert_not_equal_str(ktest_unit_t * ktest, char *expected,
                          char *actual, char * file, int line) {
    if (strcmp(expected, actual) == 0) {
        klog("[%s] ASSERTION FAILED! Expected strings to differ: \"%s\" == \"%s\" (in file %s, line %d)\n",
             ktest->name, expected, actual, file, line);
        ktest->fail_count++;
    } else {
        ktest->pass_count++;
    }
}

/* ------------------------------------------------------------------------- */

void _assert_filled(ktest_unit_t * ktest, void * buffer, uint32_t size,
                    unsigned char value, char * file, int line) {
    unsigned char * curr = (unsigned char *)buffer;
    for(uint32_t i = 0; i < size; i++) {
        if(curr[i] != value) {
            klog("[%s] ASSERTION FAILED! Buffer at 0x%x is not clear! "
                 "(in file %s, line %d)\n", ktest->name, buffer, file, line);
            ktest->fail_count++;
            break;
        }
    }
    ktest->pass_count++;
}

/* ------------------------------------------------------------------------- */

void _assert_bit_set(ktest_unit_t * ktest, unsigned int buffer,
                     unsigned int bit, char * file, int line) {
    if(!TEST_BIT(buffer, bit)) {
        klog("[%s] ASSERTION FAILED! Bit %d not set! (in file %s, line %d)\n",
             ktest->name, bit, file, line);
        ktest->fail_count++;
    } else {
        ktest->pass_count++;
    }
}

/* ------------------------------------------------------------------------- */

void ktest_run_all() {
    ktest_module_t *module;
    for (module = __start_ktest; module < __stop_ktest; module++) {
        ktest_run_module(module->name);
    }
}

/* ------------------------------------------------------------------------- */

void ktest_run_module(char * module_name) {
    ktest_module_t * module = NULL;
    for (module = __start_ktest; module < __stop_ktest; module++) {
        if(strcmp(module->name, module_name) != 0)
            break;
    }

    if(!module) {
        klog("ktest_run_module(): No module with name '%s' found!\n",
             module_name);
        return;
    }

    klog("\n");
    klog("=== Running test module: '%s' ===\n", module->name);

    if(module->module_pre)
        module->module_pre(module);

    for (uint32_t i = 0; i < module->test_count; i++) {
        /* Run pre-test set-up if necessary */
        if(module->test_pre)
            module->test_pre(module);

        /* Run the actual test function */
        klog("\n");
        klog("RUNNING TEST: %s\n", module->tests[i].name);
        klog("\n");
        module->tests[i].test_func(&module->tests[i]);

        /* Run post-test clean-up if necessary */
        if(module->test_post)
            module->test_post(module);

        if(module->tests[i].fail_count) {
            klog("[%s] TEST FAILED (%d passes, %d failures)\n",
                 module->tests[i].name, module->tests[i].pass_count,
                 module->tests[i].fail_count);
        } else {
            klog("[%s] Test succeeded! (%d passes)\n",
                 module->tests[i].name, module->tests[i].pass_count);
        }
    }

    if(module->module_post)
        module->module_post(module);

    klog("\n");
    klog("=== Module Summary ===\n");
    for (uint32_t i = 0; i < module->test_count; i++) {
        if(module->tests[i].fail_count) {
            klog("[FAIL] %s (%d passes, %d failures)\n",
                 module->tests[i].name,
                 module->tests[i].pass_count,
                 module->tests[i].fail_count);
        } else {
            klog("[ OK ] %s (%d passes)\n",
                 module->tests[i].name,
                 module->tests[i].pass_count);
        }
    }

    klog("======================\n");
    klog("\n");
}

/* ------------------------------------------------------------------------- */

void ktest_list_modules() {
    ktest_module_t *module;
    klog("Kernel Test Modules Available:\n");
    for (module = __start_ktest; module < __stop_ktest; module++) {
        klog("Module: %s\n", module->name);
    }
}

/* ------------------------------------------------------------------------- */
