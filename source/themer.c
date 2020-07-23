#include "logger.h"
#include "parser.h"
#include "themer-structs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <argp.h>

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;

	switch(key) {
		case 'd':
			if(arg == NULL)
				arguments->severity = 1;
			else
				arguments->severity = atoi(arg);
			break;
		case 'i':
			arguments->input_dirname = arg;
			break;
		case 'o':
			arguments->output_dirname = arg;
			break;
		case 'f':
			arguments->input_filename = arg;
			break;
		case 'p':
			arguments->no_post = true;
			break;
		case 't':
			arguments->theme = "light";
			arguments->input_dirname = "../test";
			arguments->output_dirname = "../out";
			arguments->no_post = true;
			break;
		case ARGP_KEY_ARG:
			if(!arguments->theme)
				arguments->theme = arg;
			else {
				logger(error, "Too many arguments!\n");
				return -1;
			}
			break;
	}

	return 0;
}

int main(int argc, char* argv[]) {
	struct argp_option options[] = {
		{ "debug", 'd', "LEVEL", OPTION_ARG_OPTIONAL, "Debug flag, default = 2, LEVEL defaults to 1 (0 = trace, 1 = debug, 2 = info, 3 = warning, 4 = error)" },
		{ "indir", 'i', "PATH", 0, "Input directory" },
		{ "outdir", 'o', "PATH", 0, "Output directory" },
		{ "file", 'f', "FILE", 0, "Input file" },
		{ "no-post", 'p', 0, 0, "Do not run post script" },
		{ "test", 't', 0, 0, "For testing" },
		{ 0 }
	};

	struct arguments arguments = {
		.theme = NULL,
		.input_filename = NULL,
		.input_dirname = NULL,
		.output_dirname = NULL,
		.severity = 2,
		.no_post = false
	};

	struct argp argp = { options, parse_opt, "THEME" };

	int ret = argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if(ret != 0)
		return ret;

	set_logger_severity_level(arguments.severity);

	if(!arguments.theme) {
		logger(error, "No theme specified\n");
		return -1;
	}

	if(arguments.input_dirname && !arguments.output_dirname) {
		logger(error, "Input directory must be supplied with an output directory\n");
		return -1;
	}

	if(arguments.input_filename) {
		if(arguments.input_dirname) {
			logger(error, "An input filename cannot be specified alongside an input directory\n");
			return -1;
		}

		if(arguments.output_dirname) {
			logger(error, "An input filename cannot be specified alongside an output directory\n");
			return -1;
		}
	}

	logger(debug, "Theme: %s\n", arguments.theme);

	if(arguments.input_dirname) {
		logger(debug, "Input directory: %s\n", arguments.input_dirname);
		logger(debug, "Output directory: %s\n", arguments.output_dirname);
	} else if(arguments.input_filename) {
		logger(debug, "Input file: %s\n", arguments.input_filename);
	}

	parser(&arguments);

	if(!arguments.no_post) {
		logger(info, "Calling themer-post script\n");
		popen("sh ~/.config/themer/themer-post.sh", "r");
		printf("here\n");
	}

	return 0;
}
