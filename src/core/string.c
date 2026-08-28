#include "string.h"

#include <stdlib.h>
#include <string.h>

String string_empty()
{
    char* new_str = malloc(1);
    new_str[0] = 0;
    return (String){.content = new_str, .length = 1, .capacity = 1};
}

String string_new(const char* str)
{
    if (str == NULL) return string_empty();

    const size_t size = strlen(str);
    if (size == 0) return string_empty();

    const size_t capacity = size + 1;

    char* new_str = malloc(capacity);
    if (new_str == NULL) return string_empty();

    strcpy(new_str, str);

    return (String){.content = new_str, .length = size, .capacity = capacity};
}


String string_new_len(const char* str, size_t length)
{
    if (str == NULL) return string_empty();

    size_t size = strlen(str);
    if (size == 0) return string_empty();

    if (size > length)
    {
        size = length;
    }
    const size_t capacity = length + 1;

    char* new_str = malloc(capacity);
    if (new_str == NULL) return string_empty();

    memcpy(new_str, str, size);
    new_str[size] = 0;

    return (String){.content = new_str, .length = size, .capacity = capacity};
}

void string_free(const String* str)
{
    if (str == NULL || str->content == NULL) return;

    free(str->content);
}
