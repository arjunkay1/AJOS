#include "string.h"

/*
 * Get the length of a null-terminated string
 */
size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/*
 * Compare two null-terminated strings
 * Returns: 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/*
 * Compare two strings up to n characters
 * Returns: 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int strncmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) {
        return 0;
    }

    while (n > 0 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }

    if (n == 0) {
        return 0;
    }

    return (unsigned char)*s1 - (unsigned char)*s2;
}

/*
 * Copy a null-terminated string from src to dest
 * Returns: pointer to dest
 */
char *strcpy(char *dest, const char *src) {
    char *original_dest = dest;

    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';

    return original_dest;
}

/*
 * Copy up to n characters from src to dest
 * If src is shorter than n, null bytes are appended
 * Returns: pointer to dest
 */
char *strncpy(char *dest, const char *src, size_t n) {
    char *original_dest = dest;

    while (n > 0 && *src != '\0') {
        *dest = *src;
        dest++;
        src++;
        n--;
    }

    /* Pad with null bytes if needed */
    while (n > 0) {
        *dest = '\0';
        dest++;
        n--;
    }

    return original_dest;
}

/*
 * Set a block of memory to a specified value
 * Returns: pointer to the memory block
 */
void *memset(void *ptr, int value, size_t size) {
    unsigned char *p = (unsigned char *)ptr;
    unsigned char v = (unsigned char)value;

    while (size > 0) {
        *p = v;
        p++;
        size--;
    }

    return ptr;
}

/*
 * Copy a block of memory from src to dest
 * Returns: pointer to dest
 */
void *memcpy(void *dest, const void *src, size_t size) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    while (size > 0) {
        *d = *s;
        d++;
        s++;
        size--;
    }

    return dest;
}
