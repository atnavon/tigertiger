#pragma once

#include "common.h"

void* file_read(const char* path, size_t* out_size);
void file_write(const char* path, void* data, size_t size);
