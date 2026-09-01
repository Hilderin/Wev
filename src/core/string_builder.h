#ifndef WEV_STRING_BUILDER_H
#define WEV_STRING_BUILDER_H
#include <stdarg.h>
#include <stddef.h>

// Contains a string builder
typedef struct StringBuilder
{
    char* buffer;
    size_t length;
    size_t capacity;
} StringBuilder;

// Create a new string builder.
StringBuilder string_builder_new();

// Free a string builder.
void string_builder_free(StringBuilder* builder);

// Append a content to a string builder.
void string_builder_append(StringBuilder* builder, const char* str);

// Append a content to a string builder with a format and args.
void string_builder_append_fmt(StringBuilder* builder, const char* format, ...);

// Append a content to a string builder with a format and a va_list.
void string_builder_append_vfmt(StringBuilder* builder, const char* format, va_list args);

#endif // WEV_STRING_BUILDER_H
