#include "parser.h"
#include "logger.h"

#include <errno.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_SIZE 255
#define MAX_PATH_LENGTH 256

#define CONCAT(a, b) a b
#define SRC_DIR "/.config/themer/src/"
#define BIN_DIR CONCAT(SRC_DIR, "../bin/")

void getfullpath(char** fullpath, const char* path, const char* filename) {
	bool endsInSlash = path[strlen(path) - 1] == '/';
	*fullpath = malloc(sizeof(char) * (strlen(path) + strlen(filename) + (endsInSlash? 1 : 2)));

	strcpy(*fullpath, path);
	if(!endsInSlash)
		strcat(*fullpath, "/");
	strcat(*fullpath, filename);
}

bool parser(const struct arguments *args) {
	char *in_path, *out_path;

	if(args->input_filename) {
		char* output_filename = malloc(sizeof(char) * (strlen(args->input_filename) + strlen(args->theme) + 2));
		strcpy(output_filename, args->input_filename);
		strcat(output_filename, ".");
		strcat(output_filename, args->theme);

		bool ret = parse_config(args->input_filename, output_filename, args->theme);

		free(output_filename);
		return ret;
	}

	if(!args->input_dirname) {
		char* home = getenv("HOME");
		in_path = malloc(sizeof(char) * (strlen(home) + strlen(SRC_DIR) + 1));
		strcpy(in_path, home);
		strcat(in_path, SRC_DIR);
	} else
		in_path = args->input_dirname;

	if(!args->output_dirname) {
		char* home = getenv("HOME");
		out_path = malloc(sizeof(char) * (strlen(home) + strlen(SRC_DIR) + 1));
		strcpy(out_path, home);
		strcat(out_path, BIN_DIR);
	} else
		out_path = args->output_dirname;

	logger(debug, "Input path: %s\n", in_path);
	logger(debug, "Output path: %s\n", out_path);

	DIR *in_dir = opendir(in_path);
	DIR *out_dir = opendir(out_path);

	if(!in_dir) {
		logger(error, "Input directory: %s\n", strerror(errno));
		return false;
	}

	if(!out_dir) {
		logger(error, "Output directory: %s\n", strerror(errno));
		return false;
	}

	struct dirent *entry;

	while((entry = readdir(in_dir)) != NULL) {
		if(entry->d_type == DT_REG) {
			char *input_filename, *output_filename;
			getfullpath(&input_filename, in_path, entry->d_name);
			getfullpath(&output_filename, out_path, entry->d_name);

			logger(debug, "Parsing: %s\n", input_filename);
			if(!parse_config(input_filename, output_filename, args->theme))
				return false;

			free(input_filename);
			free(output_filename);
		}
	}

	if(!args->input_dirname) {
		free(in_path);
		free(out_path);
	}

	closedir(in_dir);
	closedir(out_dir);

	return true;
}

bool parse_config(const char* input_filename, const char* output_filename, const char* target_theme) {
	FILE *in_file, *out_file;
	char buff[MAX_LINE_SIZE];
	char* section_theme = malloc(MAX_LINE_SIZE * sizeof(char));
	bool replace_mode = false;
	int line = 0;

	in_file = fopen(input_filename, "r");
	out_file = fopen(output_filename, "w");

	while(fgets(buff, MAX_LINE_SIZE, in_file)) {
		line++;

		size_t buff_length = strlen(buff);
		size_t pre_length = strlen("{THEME:");

		if(buff_length > 0)
			buff[buff_length - 1] = '\0';

		if(pre_length < buff_length && memcmp(buff, "{THEME:", pre_length) == 0) {
			strcpy(section_theme, buff + pre_length);
			section_theme[strlen(section_theme) - 1] = '\0';

			if(strcmp(section_theme, "ENDTHEME") != 0) {
				replace_mode = true;
				logger(trace, "%d: New section: %s\n", line, section_theme);
			} else {
				replace_mode = false;
				logger(trace, "%d: Ending section\n", line);
			}
			
			continue;
		}

		if(!replace_mode || strcasecmp(target_theme, section_theme) == 0) {
			logger(trace, "%d: writing line: %s\n", line, buff);
			fprintf(out_file, "%s\n", buff);
		}
	}

	free(section_theme);
	fclose(in_file);
	fclose(out_file);

	logger(info, "Wrote '%s' to '%s'\n", input_filename, output_filename);

	return true;
}
