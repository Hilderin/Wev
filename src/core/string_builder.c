#include "string_builder.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StringBuilder string_builder_new()
{
    return (StringBuilder){0};
}

static bool string_builder_ensure_capacity(StringBuilder* builder, const size_t capacity)
{
    if (builder == NULL) return false;
    if (builder->capacity >= capacity) return true;

    char* new_buffer = malloc(capacity);
    if (new_buffer == NULL) return false;

    if (builder->length > 0)
    {
        memcpy(new_buffer, builder->buffer, builder->length);
    }
    new_buffer[builder->length] = '\0';

    free(builder->buffer);
    builder->buffer = new_buffer;
    builder->capacity = capacity;

    return true;
}

void string_builder_append(StringBuilder* builder, const char* str)
{
    if (builder == NULL || str == NULL) return;

    const size_t len = strlen(str);
    if (len == 0) return;

    if (builder->length + len + 1 > builder->capacity)
    {
        size_t new_capacity = builder->capacity * 2;
        if (new_capacity == 0) new_capacity = len + 1;
        if (new_capacity < builder->length + len + 1) new_capacity = builder->length + len + 1;

        if (!string_builder_ensure_capacity(builder, new_capacity)) return;
    }

    memcpy(builder->buffer + builder->length, str, len);
    builder->length += len;
    builder->buffer[builder->length] = '\0';
}

void string_builder_append_vfmt(StringBuilder* builder, const char* format, va_list args)
{
    if (builder == NULL || format == NULL) return;

    // Two independent copies: vsnprintf leaves a va_list in an indeterminate state.
    va_list measure_args;
    va_copy(measure_args, args);
    const int len = vsnprintf(NULL, 0, format, measure_args);
    va_end(measure_args);

    if (len < 0) return;

    const size_t needed = builder->length + (size_t)len + 1;
    if (needed > builder->capacity)
    {
        size_t new_capacity = builder->capacity * 2;
        if (new_capacity == 0) new_capacity = needed;
        if (new_capacity < needed) new_capacity = needed;

        if (!string_builder_ensure_capacity(builder, new_capacity)) return;
    }

    va_list write_args;
    va_copy(write_args, args);
    vsnprintf(builder->buffer + builder->length, (size_t)len + 1, format, write_args);
    va_end(write_args);

    builder->length += (size_t)len;
}

void string_builder_append_fmt(StringBuilder* builder, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    string_builder_append_vfmt(builder, format, args);
    va_end(args);
}