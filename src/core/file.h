#ifndef WEV_FILE_H
#define WEV_FILE_H
#include <stddef.h>

#include "string.h"

// Returns an empty string.
String string_empty();

// Returns the size of a file. Returns 0 if the file does not exists.
size_t file_len(const char* path);

// Reads the content of a file into the string.
char* file_read_str(const char* path);

#endif // WEV_FILE_H
