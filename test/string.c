/*
 * kernel/test/string.c
 * String Functions Testing
 */

#include <rotary/test/string.h>

/* ------------------------------------------------------------------------- */
/* Test Set-up and Clean-up                                                  */
/* ------------------------------------------------------------------------- */

int32_t string_pre_module(ktest_module_t * module) {
    // Any setup before the module tests run
    return 0;
}

/* ------------------------------------------------------------------------- */

int32_t string_post_module(ktest_module_t * module) {
    // Any cleanup after the module tests run
    return 0;
}

/* ------------------------------------------------------------------------- */

int32_t string_pre_test(ktest_module_t * module) {
    // Any setup before each test
    return 0;
}

/* ------------------------------------------------------------------------- */

int32_t string_post_test(ktest_module_t * module) {
    // Any cleanup after each test
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Utility Functions                                                         */
/* ------------------------------------------------------------------------- */

// Define MAX_STRING_LEN if not already defined
#ifndef MAX_STRING_LEN
#define MAX_STRING_LEN 1024
#endif

/* ------------------------------------------------------------------------- */
/* Unit Tests                                                                */
/* ------------------------------------------------------------------------- */

void string_test_memset(ktest_unit_t * ktest) {
    char buffer[64];

    /* Fill with zero and test */
    memset(buffer, 0, sizeof(buffer));
    assert_filled(buffer, sizeof(buffer), 0);

    /* Fill with 0xFF bytes and test */
    memset(buffer, 0xFF, sizeof(buffer));
    assert_filled(buffer, sizeof(buffer), 0xFF);

    /* Partial fill with 0xFF, check, and ensure other half still NULL */
    memset(buffer, 0xFF, sizeof(buffer) / 2);
    assert_filled(buffer, sizeof(buffer) / 2, 0xFF);
    assert_filled(buffer + sizeof(buffer) / 2, sizeof(buffer) / 2, 0x00);
}

/* ------------------------------------------------------------------------- */

void string_test_memcpy(ktest_unit_t * ktest) {
    char src[128];
    char dst[128];

    /* Initialise buffers with known data */
    memset(src, 'A', sizeof(src));
    memset(dst, 0, sizeof(dst));

    /* Copy all of src to dst */
    memcpy(dst, src, sizeof(src));
    assert_filled(dst, sizeof(dst), 'A');

    /* Update source buffer with 'B' */
    memset(src, 'B', sizeof(src));
    
    /* Partial copy */
    memcpy(dst, src, sizeof(src) / 2);
    assert_filled(dst, sizeof(src) / 2, 'B');
    assert_filled(dst + sizeof(src) / 2, sizeof(src) / 2, 'A');
}

/* ------------------------------------------------------------------------- */

void string_test_int_to_str(ktest_unit_t * ktest) {
    char buffer[32];
   
    /* Test positive number */
    int_to_str(12345, buffer);
    assert_equal(buffer, "12345");
}

/* ------------------------------------------------------------------------- */

/* Test uint_to_str() */
void string_test_uint_to_str(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test int_to_hex_str() */
void string_test_int_to_hex_str(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test fill_buffer() */
void string_test_fill_buffer(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test sprintf() */
void string_test_sprintf(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test strcmp() */
void string_test_strcmp(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test strcpy() */
void string_test_strcpy(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test strncpy() */
void string_test_strncpy(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test strcat() */
void string_test_strcat(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test strlen() */
void string_test_strlen(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test reverse() */
void string_test_reverse(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test itoa() */
void string_test_itoa(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

/* Test atoi() */
void string_test_atoi(ktest_unit_t * ktest) {
}

/* ------------------------------------------------------------------------- */

/* Test format_size() */
void string_test_format_size(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */
/* Test Registration                                                         */
/* ------------------------------------------------------------------------- */

static ktest_unit_t test_units[] = {
    KTEST_UNIT("string-test-memset", string_test_memset),
    KTEST_UNIT("string-test-memcpy", string_test_memcpy),
    KTEST_UNIT("string-test-int-to-str", string_test_int_to_str),
    KTEST_UNIT("string-test-uint-to-str", string_test_uint_to_str),
    KTEST_UNIT("string-test-int-to-hex-str", string_test_int_to_hex_str),
    KTEST_UNIT("string-test-fill-buffer", string_test_fill_buffer),
    KTEST_UNIT("string-test-sprintf", string_test_sprintf),
    KTEST_UNIT("string-test-strcmp", string_test_strcmp),
    KTEST_UNIT("string-test-strcpy", string_test_strcpy),
    KTEST_UNIT("string-test-strncpy", string_test_strncpy),
    KTEST_UNIT("string-test-strcat", string_test_strcat),
    KTEST_UNIT("string-test-strlen", string_test_strlen),
    KTEST_UNIT("string-test-reverse", string_test_reverse),
    KTEST_UNIT("string-test-itoa", string_test_itoa),
    KTEST_UNIT("string-test-atoi", string_test_atoi),
    KTEST_UNIT("string-test-format-size", string_test_format_size),
};

KTEST_MODULE_DEFINE("string", test_units,
                    string_pre_module,
                    string_post_module,
                    string_pre_test,
                    string_post_test);

/* ------------------------------------------------------------------------- */

