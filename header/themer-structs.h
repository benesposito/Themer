#pragma once

#include <stdbool.h>

struct arguments {
	char* theme;
	char *input_dirname, *output_dirname;
	char* input_filename;
	int severity;
	bool no_post;
};
