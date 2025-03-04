/*
 * include/rotary/string.h
 * String and Memory Operations
 */

#ifndef INC_STRING_H
#define INC_STRING_H

#include <rotary/core.h>
#include <rotary/debug.h>

/* ------------------------------------------------------------------------- */

#define MAX_STRING_LEN 512

/* ------------------------------------------------------------------------- */

void     int_to_str(int32_t num, char *dest);
void     int_to_hex_str(uint32_t num, char *dest);

void *   memset(void *dest, int value, size_t n);
void *   memcpy(void *dest, const void *src, size_t n);

int      strcmp(const char *str1, const char *str2);
char *   strcpy(char *dest_buf, const char *src_buf);
char *   strncpy(char *dest_buf, const char *src_buf, size_t n);
char *   strcat(char *str1, const char *str2);
int      sprintf(char *dest_buf, const char *format_str, ...);
size_t   strlen(const char *string);
void     itoa(uint32_t num, char *dec_string);
int      atoi(const char *str);

/* ------------------------------------------------------------------------- */

#endif

