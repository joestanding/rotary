/*
 * kernel/test/string.c
 * String Functions Testing
 */

#include <rotary/test/string.h>
#include <rotary/string.h>

/* ------------------------------------------------------------------------- */
/* Test Set-up and Clean-up                                                  */
/* ------------------------------------------------------------------------- */

int32_t string_pre_module(ktest_module_t * module) {
    /* Any setup before the module tests run */
    return 0;
}

int32_t string_post_module(ktest_module_t * module) {
    /* Any cleanup after the module tests run */
    return 0;
}

int32_t string_pre_test(ktest_module_t * module) {
    /* Any setup before each test */
    return 0;
}

int32_t string_post_test(ktest_module_t * module) {
    /* Any cleanup after each test */
    return 0;
}

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

    /* Partial fill with 0xFF, then verify remaining bytes remain 0 */
    memset(buffer, 0xFF, sizeof(buffer) / 2);
    assert_filled(buffer, sizeof(buffer) / 2, 0xFF);
    assert_filled(buffer + sizeof(buffer) / 2, sizeof(buffer) / 2, 0x00);
}

void string_test_memcpy(ktest_unit_t * ktest) {
    char src[128];
    char dst[128];

    /* Initialize buffers with known data */
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

void string_test_int_to_str(ktest_unit_t * ktest) {
    char buffer[32];
    
    /* Test positive number */
    int_to_str(12345, buffer);
    assert_equal(buffer, "12345");

    /* Test zero */
    int_to_str(0, buffer);
    assert_equal(buffer, "0");

    /* Test negative number */
    int_to_str(-6789, buffer);
    assert_equal(buffer, "-6789");
}

void string_test_uint_to_str(ktest_unit_t * ktest) {
    char buffer[32];

    /* Test zero */
    uint_to_str(0, buffer);
    assert_equal(buffer, "0");

    /* Test a typical unsigned number */
    uint_to_str(123456, buffer);
    assert_equal(buffer, "123456");

    /* Test maximum 32-bit unsigned number */
    uint_to_str(4294967295U, buffer);
    assert_equal(buffer, "4294967295");
}

void string_test_int_to_hex_str(ktest_unit_t * ktest) {
    char buffer[16];

    /* Test zero */
    int_to_hex_str(0, buffer);
    assert_equal(buffer, "00000000");

    /* Test a known hex value */
    int_to_hex_str(0x1234ABCD, buffer);
    assert_equal(buffer, "1234abcd");

    /* Test maximum hex value */
    int_to_hex_str(0xFFFFFFFF, buffer);
    assert_equal(buffer, "ffffffff");
}

void string_test_fill_buffer(ktest_unit_t * ktest) {
    char buffer[20] = {0};
    uint32_t index = 0;

    /* Fill first 10 characters with 'X' */
    fill_buffer(buffer, &index, 'X', 10);
    assert_equal(index, 10);
    for (uint32_t i = 0; i < 10; i++) {
        assert_equal(buffer[i], 'X');
    }
    /* Verify remaining bytes are still zero */
    for (uint32_t i = 10; i < 20; i++) {
        assert_equal(buffer[i], 0);
    }
}

void string_test_sprintf(ktest_unit_t * ktest) {
    char buffer[128];

    /* Test simple string without format specifiers */
    sprintf(buffer, "Hello World");
    assert_equal(buffer, "Hello World");

    /* Test %s specifier */
    sprintf(buffer, "String: %s", "Test");
    assert_equal(buffer, "String: Test");

    /* Test %d specifier with padding */
    sprintf(buffer, "Number: %5d", 42);
    assert_equal(buffer, "Number: 00042");

    /* Test %u specifier with padding */
    sprintf(buffer, "Unsigned: %4u", 7);
    assert_equal(buffer, "Unsigned: 0007");

    /* Test %x specifier with padding */
    sprintf(buffer, "Hex: %8x", 0xABCD);
    assert_equal(buffer, "Hex: 0000abcd");

    /* Test %c specifier with padding */
    sprintf(buffer, "Char: %3c", 'Z');
    assert_equal(buffer, "Char: 00Z");

    /* Test combined specifiers */
    sprintf(buffer, "%s %d %u %x %c", "Mix", -5, 123, 0xF, 'Q');
    assert_equal(buffer, "Mix -5 123 f Q");
}

void string_test_strcmp(ktest_unit_t * ktest) {
    int rv = strcmp("EQUAL", "EQUAL");
    assert_equal(rv, 0);

    rv = strcmp("ABC", "ABD");
    assert(rv < 0);

    rv = strcmp("XYZ", "XYA");
    assert(rv > 0);

    rv = strcmp("", "");
    assert_equal(rv, 0);
}

void string_test_strcpy(ktest_unit_t * ktest) {
    char src[] = "Copy this string";
    char dest[64] = {0};
    strcpy(dest, src);
    assert_equal(dest, src);
}

void string_test_strncpy(ktest_unit_t * ktest) {
    char src[] = "Source";
    char dest[16];

    /* When n >= strlen(src): expect full copy with padding */
    strncpy(dest, src, 10);
    assert_equal(dest, "Source");

    /* When n < strlen(src): expect truncated result */
    char dest2[16];
    strncpy(dest2, src, 3);
    /* Our implementation pads with nulls; thus, dest2 should be "Sou" */
    assert_equal(dest2, "Sou");
}

void string_test_strcat(ktest_unit_t * ktest) {
    char buffer[64] = "Hello";
    strcat(buffer, " World");
    assert_equal(buffer, "Hello World");
}

void string_test_strlen(ktest_unit_t * ktest) {
    int len = strlen("Test string");
    assert_equal(len, 11);

    len = strlen("");
    assert_equal(len, 0);
}

void string_test_reverse(ktest_unit_t * ktest) {
    char str1[] = "abcdef";
    reverse(str1, 6);
    assert_equal(str1, "fedcba");

    char str2[] = "a";
    reverse(str2, 1);
    assert_equal(str2, "a");
}

void string_test_itoa(ktest_unit_t * ktest) {
    char buffer[16];

    itoa(0, buffer);
    assert_equal(buffer, "0");

    itoa(123456, buffer);
    assert_equal(buffer, "123456");

    itoa(987654321, buffer);
    assert_equal(buffer, "987654321");
}

void string_test_atoi(ktest_unit_t * ktest) {
    int value = atoi("12345");
    assert_equal(value, 12345);

    value = atoi("-6789");
    assert_equal(value, -6789);

    value = atoi("+42");
    assert_equal(value, 42);

    value = atoi("0");
    assert_equal(value, 0);
}

void string_test_format_size(ktest_unit_t * ktest) {
    char buffer[32];

    /* Test for bytes less than 1024 */
    format_size(512, buffer);
    assert_equal(buffer, "512B");

    /* Test for kilobytes */
    format_size(2048, buffer);
    assert_equal(buffer, "2KB");

    /* Test for megabytes (3 MB: 3 * 1024 * 1024 = 3145728) */
    format_size(3145728, buffer);
    assert_equal(buffer, "3MB");
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

