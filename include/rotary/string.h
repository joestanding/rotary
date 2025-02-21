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

void     int_to_str(int32_t num, char * dest);
void     int_to_hex_str(uint32_t num, char * dest);

void *   memset(void * dest, int value, size_t n);
void *   memcpy(void * dest, void * src, uint32_t size);

int      strcmp(const char * str1, const char * str2);
void     strncpy(char * dest_buf, char * src_buf, uint32_t n);
char *   strcat(char * str1, char * str2);
void     sprintf(char * dest_buf, char * format_str, ...);
uint32_t strlen(const char * string);
void     itoa(uint32_t num, char * dec_string);
int      atoi(char * str);

/* ------------------------------------------------------------------------- */

#endif
