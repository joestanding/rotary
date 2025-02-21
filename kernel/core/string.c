/*
 * kernel/string.c
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

void * memcpy(void * dest, void * src, uint32_t size) {
    char * d = dest;
    char * s = src;
    while (size--)
        *d++ = *s++;
    return dest;
}

/* ------------------------------------------------------------------------- */

void int_to_str(int32_t num, char * dest)
{
    int i = 0;
    int isNegative = 0;

    // Handle 0 explicitly, otherwise empty string is printed for 0
    if (num == 0)
    {
        dest[i++] = '0';
        dest[i] = '\0';
        return;
    }

    // Check if number is negative
    if (num < 0)
    {
        isNegative = 1;
        num = -num; // Make number positive
    }

    // Process individual digits
    while (num != 0)
    {
        int rem = num % 10;
        dest[i++] = rem + '0';
        num = num / 10;
    }

    // If number is negative, append '-'
    if (isNegative)
        dest[i++] = '-';

    // Terminate string
    dest[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end)
    {
        char temp = dest[start];
        dest[start] = dest[end];
        dest[end] = temp;
        start++;
        end--;
    }
}


/* ------------------------------------------------------------------------- */

void uint_to_str(uint32_t num, char * dest)
{
    int i = 0;

    // Handle 0 explicitly, otherwise empty string is printed for 0
    if (num == 0)
    {
        dest[i++] = '0';
        dest[i] = '\0';
        return;
    }

    // Process individual digits
    while (num != 0)
    {
        int rem = num % 10;
        dest[i++] = rem + '0';
        num = num / 10;
    }

    // Terminate string
    dest[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end)
    {
        char temp = dest[start];
        dest[start] = dest[end];
        dest[end] = temp;
        start++;
        end--;
    }
}

/* ------------------------------------------------------------------------- */

void int_to_hex_str(uint32_t num, char * dest) {
  int i;
  char hex_digits[] = "0123456789abcdef";
  
  for (i = 0; i < 8; i++) {
    dest[7-i] = hex_digits[num & 0xf];
    num >>= 4;
  }
  dest[8] = '\0';
}

/* ------------------------------------------------------------------------- */

void fill_buffer(char* dest_buf, uint32_t * dest_index, char c, int count) {
    for (int i = 0; i < count; i++) {
        dest_buf[(*dest_index)++] = c;
    }
}

/* ------------------------------------------------------------------------- */

void sprintf(char *dest_buf, char *format_str, ...) {
    __builtin_va_list list;
    __builtin_va_start(list, format_str);

    uint32_t index = 0;
    uint32_t processed = 0;
    uint32_t dest_index = 0;

    // Iterate until the end of the string
    while (format_str[index] != '\0') {
        if (format_str[index] == '%') {
            char num[32];
            memset(&num, 0, sizeof(num));

            // Copy characters before the format specifier
            while (processed < index) {
                dest_buf[dest_index++] = format_str[processed++];
            }

            index++; // move past '%'

            // Padding
            int padding = 0;
            while(format_str[index] >= '0' && format_str[index] <= '9') {
                padding = padding * 10 + (format_str[index] - '0');
                index++;
            }

            int length = 0;

            switch (format_str[index]) {
                case 's':
                    {
                        char *str_arg = __builtin_va_arg(list, char*);
                        // Padding
                        length = strlen(str_arg);
                        if (padding > length) {
                            fill_buffer(dest_buf, &dest_index, '0', padding - length);
                        }
                        // Concatenate the string argument
                        while (*str_arg != '\0') {
                            dest_buf[dest_index++] = *str_arg++;
                        }
                    }
                    break;
                case 'd':
                    int_to_str(__builtin_va_arg(list, int), (char*)&num);
                    // Padding
                    length = strlen(num);
                    if (padding > length) {
                        fill_buffer(dest_buf, &dest_index, '0', padding - length);
                    }
                    // Concatenate the decimal number
                    char *num_ptr = num;
                    while (*num_ptr != '\0') {
                        dest_buf[dest_index++] = *num_ptr++;
                    }
                    break;
                case 'u':
                    uint_to_str(__builtin_va_arg(list, unsigned int), num);
                    // Padding
                    length = strlen(num);
                    if (padding > length) {
                        fill_buffer(dest_buf, &dest_index, '0', padding - length);
                    }
                    // Concatenate the unsigned decimal number
                    char *unum_ptr = num;
                    while (*unum_ptr != '\0') {
                        dest_buf[dest_index++] = *unum_ptr++;
                    }
                    break;
                case 'x':
                    int_to_hex_str(__builtin_va_arg(list, int), num);
                    // Padding
                    length = strlen(num);
                    if (padding > length) {
                        fill_buffer(dest_buf, &dest_index, '0', padding - length);
                    }
                    // Concatenate the hexadecimal number
                    char *hex_ptr = num;
                    while (*hex_ptr != '\0') {
                        dest_buf[dest_index++] = *hex_ptr++;
                    }
                    break;
                case 'c':
                    // Padding
                    if (padding > 1) {
                        fill_buffer(dest_buf, &dest_index, '0', padding - 1);
                    }
                    // Concatenate the character
                    dest_buf[dest_index++] = __builtin_va_arg(list, int);
                    break;
            }

            processed = index + 1; // index is already moved past the format character
        }
        index++;
    }

    // Copy characters after the last format specifier
    while (processed < index) {
        dest_buf[dest_index++] = format_str[processed++];
    }
    
    dest_buf[dest_index] = '\0'; // Null-terminating the destination buffer
    __builtin_va_end(list);
}

/* ------------------------------------------------------------------------- */

int strcmp(const char * str1, const char * str2) {
    if(strlen(str1) != strlen(str2))
        return 0;

    for(uint32_t i = 0; i < strlen(str1); i++ ) {
        if(str1[i] != str2[i]) {
            return 0;
        }
    }

    return 1;
}

/* ------------------------------------------------------------------------- */

void strcpy(char * dest_buf, char * src_buf) {
    uint32_t i = 0;
    while(*(dest_buf+i) != 0x00) {
        *(dest_buf+i) = src_buf[i];
        i++;
    }
    *(dest_buf + ++i) = 0x00;
}

/* ------------------------------------------------------------------------- */

void strncpy(char * dest_buf, char * src_buf, uint32_t n) {
    for(uint32_t i = 0; i < n; i++) {
        *(dest_buf+i) = src_buf[i];
    }
}

/* ------------------------------------------------------------------------- */

char * strcat(char * str1, char * str2) {
    uint32_t str1_len = strlen(str1);
    uint32_t str2_len = strlen(str2);
    uint32_t i;
    for(i = 0; i < str2_len; i++) {
        str1[str1_len+i] = str2[i];
    }
    str1[str1_len+i] = '\0';
    return str1;
}

/* ------------------------------------------------------------------------- */

uint32_t strlen(const char * string) {
    for(uint32_t i = 0; i < MAX_STRING_LEN; i++) {
        if(string[i] == '\0')
            return i;
    }
    return -1;
}

/* ------------------------------------------------------------------------- */

// https://www.geeksforgeeks.org/implement-itoa/

void reverse(char * str, int length) {
	int start = 0;
	int end = length - 1;
	while(start < end) {
        uint8_t temp = *(str+start);
		*(str+start) = *(str+end);
        *(str+end) = temp;
		start++;
		end--;
	}
}

/* ------------------------------------------------------------------------- */

void itoa(uint32_t num, char* dec_string) {
    int i;
    uint32_t divisor = 1000000000;
    int leading_zeroes = 1;
    for (i = 0; i < 10; i++) {
        int digit = num / divisor;
        if (digit != 0 || !leading_zeroes) {
            dec_string[i] = digit + '0';
            leading_zeroes = 0;
        }
        num %= divisor;
        divisor /= 10;
    }
    dec_string[10] = '\0';
}

/* ------------------------------------------------------------------------- */

int atoi(char * str) {
    int res = 0;
    for(int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';
    return res;
}

/* ------------------------------------------------------------------------- */

void format_size(uint32_t bytes, char *dest) {
    char *suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int suffix_index = 0;
    uint32_t base = 1024;
    uint32_t whole_part, fraction_part;

    // Reduce bytes progressively until they fit the correct unit
    while (bytes >= base && suffix_index < 4) {
        bytes /= base;
        suffix_index++;
    }

    // Whole part is just the reduced bytes
    whole_part = bytes;

    // Handle fraction part for more precision
    fraction_part = ((bytes * 1024) % base) * 10 / base;

    // Format into the destination buffer (e.g., "4.2KB")
    int_to_str(whole_part, dest);
    int len = strlen(dest);

    // Add the fractional part only if it's non-zero
    if (fraction_part > 0) {
        dest[len++] = '.';
        dest[len++] = '0' + fraction_part; // Add fractional part
    }

    dest[len] = '\0'; // Null-terminate the string

    // Add the appropriate unit suffix
    strcat(dest, suffixes[suffix_index]);
}

/* ------------------------------------------------------------------------- */

#include "test/string.c"

/* ------------------------------------------------------------------------- */
