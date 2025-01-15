#pragma once

#include <stdio.h>

char* read_file_into_char(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        log_error("failed to open file");
        return NULL;
    }
 
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
 
    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        log_error("memory alloc failed");
        fclose(file);
        return NULL;
    }
 
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        log_error("error reading file");
        free(buffer);
        fclose(file);
        return NULL;
    }
 
    buffer[bytes_read] = '\0';
 
    fclose(file);
    return buffer;
}