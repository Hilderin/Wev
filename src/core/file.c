#include "file.h"

#include <stdio.h>
#include <stdlib.h>

size_t file_len(const char* path)
{
    if (path == NULL) return 0;

    FILE* fptr = fopen(path, "r");
    if (fptr == NULL) return 0;

    size_t size = 0;
    if (fseek(fptr, 0, SEEK_END) == 0)
    {
        const long end = ftell(fptr);
        if (end > 0)
        {
            size = (size_t)end;
        }
    }

    fclose(fptr);

    return size;
}

char* file_read_str(const char* path)
{
    if (path == NULL) return 0;

    FILE* fptr = fopen(path, "r");
    if (fptr == NULL) return 0;

    size_t size = 0;
    if (fseek(fptr, 0, SEEK_END) != 0)
    {
        fclose(fptr);
        return NULL;
    }
    else
    {
        const long end = ftell(fptr);
        if (end < 0)
        {
            fclose(fptr);
            return NULL;
        }
        size = (size_t)end;

        // Return the file at the start.
        fseek(fptr, 0, SEEK_SET);
    }

    // Read all the file content.
    char* buffer = malloc(size + 1);
    if (buffer == NULL)
    {
        fclose(fptr);
        return NULL;
    }
    // fread never returns more than the requested count (size), so buffer[read]
    // is always within the size + 1 allocation. The ArrayBound analyzer cannot
    // prove this and reports a false positive; read is bounded by fread's count.
    const size_t read = fread(buffer, 1, size, fptr);
    buffer[read] = '\0'; // NOLINT(clang-analyzer-security.ArrayBound)

    fclose(fptr);

    return buffer;
}
