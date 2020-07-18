#pragma once

#include "themer-structs.h"

#include <stdbool.h>
#include <dirent.h>

bool parser(const struct arguments *args);
bool parse_config(const char* input_filename, const char* output_filename, const char* target_theme);
