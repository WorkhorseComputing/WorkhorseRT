#include <stdWorkhorse.h>

inline
void *memcpy(void *dest, const void *src, size_t n)
{
    const char *_src = (char *)src;
    char *_dest = (char *)dest;

    for (uint64_t i = 0; i < n; i++)
        _dest[i] = _src[i]; 

    return dest;
}

inline
void *memset(void *str, int c, size_t n)
{
    char *_str = (char *)str;

    for (uint64_t i = 0; i < n; i++)
        _str[i] = c;

    return str;
}