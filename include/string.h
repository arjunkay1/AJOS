#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

/* Get the length of a string */
size_t strlen(const char *str);

/* Compare two strings */
int strcmp(const char *s1, const char *s2);

/* Compare two strings up to n characters */
int strncmp(const char *s1, const char *s2, size_t n);

/* Copy a string */
char *strcpy(char *dest, const char *src);

/* Copy up to n characters of a string */
char *strncpy(char *dest, const char *src, size_t n);

/* Set memory to a value */
void *memset(void *ptr, int value, size_t size);

/* Copy memory */
void *memcpy(void *dest, const void *src, size_t size);

#endif /* STRING_H */
