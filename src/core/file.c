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
        size = ftell(fptr);

        // Return the file at the start.
        fseek(fptr, 0, SEEK_SET);
    }

    // Read all the file content.
    char* buffer = malloc(size + 1);
    size = fread(buffer, 1, size, fptr);
    buffer[size] = '\0';

    fclose(fptr);

    return buffer;
}
