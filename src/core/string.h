#ifndef WEV_STRING_H
#define WEV_STRING_H
#include <stddef.h>
#include <stdint.h>

// Contains a string
typedef struct String
{
    char* content;
    size_t length;
    size_t capacity;
} String;

// Create a new string.
String string_new(const char* str);

// Create a new string with a length.
String string_new_len(const char* str, size_t length);

// Free a string
void string_free(String* str);

// Concat 2 strings
String string_concat(const char* str_a, const char* str_b);

#endif // WEV_STRING_H
