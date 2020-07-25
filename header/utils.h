#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <string.h>

void join(char* str, char* delim, size_t argc, ...);
void getpath(char* path, size_t argc, ...);
