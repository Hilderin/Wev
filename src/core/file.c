#include "file.h"

#include <stdio.h>
#include <stdlib.h>

size_t file_len(const char* path)
{
    if (path == NULL) return 0;

    FILE* fptr = fopen(path, "r");
    if (fptr == NULL) return 0;

    long size = 0;
    if (fseek(fptr, 0, SEEK_END) == 0)
    {
        size = ftell(fptr);
    }

    fclose(fptr);

    return size;
}

String file_read_str(const char* path)
{
    size_t size = file_len(path);

    if (size == 0) return string_empty();

    const char* buffer = (char*)malloc(size);

    return string_new_len(buffer, size);
}
