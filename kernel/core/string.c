/*
 * kernel/core/string.c
 * String Processing Functions
 *
 * Basic functions for string operations, such as copying.
 */

#include <rotary/string.h>

/* ------------------------------------------------------------------------- */

void * memset(void *dest, int value, size_t n) {
    unsigned char *ptr = dest;
    while (n--) {
        *ptr++ = (unsigned char)value;
    }
    return dest;
}

/* ------------------------------------------------------------------------- */

void * memcpy(void *dest, const void *src, size_t n) {
    const unsigned char *s = src;
    unsigned char *d = dest;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

/* ------------------------------------------------------------------------- */

void int_to_str(int32_t num, char *dest) {
    int i = 0;
    int isNegative = 0;

    if (num == 0) {
        dest[i++] = '0';
        dest[i] = '\0';
        return;
    }

    if (num < 0) {
        isNegative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % 10;
        dest[i++] = rem + '0';
        num = num / 10;
    }

    if (isNegative)
        dest[i++] = '-';

    dest[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = dest[start];
        dest[start] = dest[end];
        dest[end] = temp;
        start++;
        end--;
    }
}

/* ------------------------------------------------------------------------- */

void uint_to_str(uint32_t num, char *dest) {
    int i = 0;
    if (num == 0) {
        dest[i++] = '0';
        dest[i] = '\0';
        return;
    }
    while (num != 0) {
        int rem = num % 10;
        dest[i++] = rem + '0';
        num = num / 10;
    }
    dest[i] = '\0';
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = dest[start];
        dest[start] = dest[end];
        dest[end] = temp;
        start++;
        end--;
    }
}

/* ------------------------------------------------------------------------- */

void int_to_hex_str(uint32_t num, char *dest) {
    char hex_digits[] = "0123456789abcdef";
    char temp[9]; /* Max 8 hex digits + null terminator */
    int i = 0;
    int leading = 1; /* Flag to skip leading zeroes */

    if (num == 0) { /* Special case: Zero should be "0" */
        dest[0] = '0';
        dest[1] = '\0';
        return;
    }

    for (int j = 7; j >= 0; j--) {
        char digit = hex_digits[(num >> (j * 4)) & 0xF];
        if (digit != '0' || !leading) {
            temp[i++] = digit;
            leading = 0; /* Found the first non-zero digit, stop skipping */
        }
    }

    temp[i] = '\0';
    strcpy(dest, temp);
}

/* ------------------------------------------------------------------------- */

void fill_buffer(char *dest_buf, uint32_t *dest_index, char c, int count) {
    for (int i = 0; i < count; i++) {
        dest_buf[(*dest_index)++] = c;
    }
}

/* ------------------------------------------------------------------------- */

int sprintf(char *dest_buf, const char *format_str, ...) {
    __builtin_va_list list;
    __builtin_va_start(list, format_str);

    uint32_t index = 0;
    uint32_t processed = 0;
    uint32_t dest_index = 0;

    while (format_str[index] != '\0') {
        if (format_str[index] == '%') {
            char num[32];
            memset(num, 0, sizeof(num));

            // Copy literal text before the format specifier.
            while (processed < index) {
                dest_buf[dest_index++] = format_str[processed++];
            }

            index++; // Move past '%'

            // Process optional padding.
            int padding = 0;
            while (format_str[index] >= '0' && format_str[index] <= '9') {
                padding = padding * 10 + (format_str[index] - '0');
                index++;
            }

            int length = 0;

            switch (format_str[index]) {
                case 's': {
                        char *str_arg = __builtin_va_arg(list, char*);
                        length = strlen(str_arg);
                        if (padding > length) {
                            fill_buffer(dest_buf, &dest_index, '0', padding - length);
                        }
                        while (*str_arg != '\0') {
                            dest_buf[dest_index++] = *str_arg++;
                        }
                    }
                    break;
                case 'd': {
                        int_to_str(__builtin_va_arg(list, int), num);
                        length = strlen(num);
                        if (padding > length) {
                            fill_buffer(dest_buf, &dest_index, '0', padding - length);
                        }
                        char *num_ptr = num;
                        while (*num_ptr != '\0') {
                            dest_buf[dest_index++] = *num_ptr++;
                        }
                    }
                    break;
                case 'u': {
                        uint_to_str(__builtin_va_arg(list, unsigned int), num);
                        length = strlen(num);
                        if (padding > length) {
                            fill_buffer(dest_buf, &dest_index, '0', padding - length);
                        }
                        char *unum_ptr = num;
                        while (*unum_ptr != '\0') {
                            dest_buf[dest_index++] = *unum_ptr++;
                        }
                    }
                    break;
                case 'x': {
                        int_to_hex_str(__builtin_va_arg(list, int), num);
                        length = strlen(num);
                        if (padding > length) {
                            fill_buffer(dest_buf, &dest_index, '0', padding - length);
                        }
                        char *hex_ptr = num;
                        while (*hex_ptr != '\0') {
                            dest_buf[dest_index++] = *hex_ptr++;
                        }
                    }
                    break;
                case 'c': {
                        if (padding > 1) {
                            fill_buffer(dest_buf, &dest_index, '0', padding - 1);
                        }
                        dest_buf[dest_index++] = __builtin_va_arg(list, int);
                    }
                    break;
                default:
                    // For unknown specifiers, copy the character as is.
                    dest_buf[dest_index++] = format_str[index];
                    break;
            }

            processed = index + 1;
        }
        index++;
    }

    // Copy any remaining characters.
    while (processed < index) {
        dest_buf[dest_index++] = format_str[processed++];
    }
    
    dest_buf[dest_index] = '\0';
    __builtin_va_end(list);
    return dest_index;
}

/* ------------------------------------------------------------------------- */

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return ((unsigned char)*s1 - (unsigned char)*s2);
}

/* ------------------------------------------------------------------------- */

char * strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

/* ------------------------------------------------------------------------- */

char * strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}

/* ------------------------------------------------------------------------- */

char * strcat(char *str1, const char *str2) {
    size_t str1_len = strlen(str1);
    size_t i;
    for (i = 0; str2[i] != '\0'; i++) {
        str1[str1_len + i] = str2[i];
    }
    str1[str1_len + i] = '\0';
    return str1;
}

/* ------------------------------------------------------------------------- */

size_t strlen(const char *string) {
    size_t i = 0;
    while (i < MAX_STRING_LEN && string[i] != '\0') {
        i++;
    }
    return i;
}

/* ------------------------------------------------------------------------- */

void reverse(char *str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        uint8_t temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

/* ------------------------------------------------------------------------- */

void itoa(uint32_t num, char *dec_string) {
    if (num == 0) {
        dec_string[0] = '0';
        dec_string[1] = '\0';
        return;
    }
    int i = 0;
    for (uint32_t divisor = 1000000000; divisor > 0; divisor /= 10) {
        int digit = num / divisor;
        if (digit || i > 0) {
            dec_string[i++] = digit + '0';
        }
        num %= divisor;
    }
    dec_string[i] = '\0';
}

/* ------------------------------------------------------------------------- */

int atoi(const char *str) {
    int res = 0;
    int sign = 1;
    int i = 0;
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }
    for (; str[i] != '\0'; i++) {
        res = res * 10 + (str[i] - '0');
    }
    return sign * res;
}

/* ------------------------------------------------------------------------- */

void format_size(uint32_t bytes, char *dest) {
    char *suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int suffix_index = 0;
    uint32_t base = 1024;
    uint32_t whole_part, fraction_part;

    while (bytes >= base && suffix_index < 4) {
        bytes /= base;
        suffix_index++;
    }

    whole_part = bytes;
    fraction_part = ((bytes * 1024) % base) * 10 / base;

    int_to_str(whole_part, dest);
    int len = strlen(dest);

    if (fraction_part > 0) {
        dest[len++] = '.';
        dest[len++] = '0' + fraction_part;
    }

    dest[len] = '\0';
    strcat(dest, suffixes[suffix_index]);
}

/* ------------------------------------------------------------------------- */

#include "test/string.c"

/* ------------------------------------------------------------------------- */

