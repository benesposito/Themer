#include "parser.h"
#include "logger.h"

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

#define MAX_LINE_LENGTH 255
#define MAX_PATH_LENGTH 256
#define MAX_HEADER_VALUE_LENGTH 30

#define CONCAT(a, b) a b
#define SRC_DIR "/.config/themer/src/"
#define BIN_DIR CONCAT(SRC_DIR, "../bin/")

#define THEME_END_VALUE "ENDTHEME"

#define COLORS_PATH 

enum header_type { theme, color, unknown };

struct color_entry {
	char* name;
	char* color;
};

static char* HOME;
static struct color_entry colors[50];

static void getfullpath(char* fullpath, const char* path, const char* filename) {
	bool endsInSlash = path[strlen(path) - 1] == '/';

	strcpy(fullpath, path);
	if(!endsInSlash)
		strcat(fullpath, "/");
	strcat(fullpath, filename);
}

static bool parse_line(const char* line, char* pre, enum header_type *header, char* value, char* post) {
	char* s_header;

	// Get matching groups from 'line'
	regex_t regex;
	regcomp(&regex, "(.*?)\\{(\\w*):(\\w*)\\}(.*)", REG_EXTENDED);
	size_t n_groups = regex.re_nsub + 1;
	regmatch_t *groups = malloc(n_groups * sizeof(regmatch_t));
	int ret = regexec(&regex, line, n_groups, groups, 0);
	regfree(&regex);

	if(ret == 0) {
		char** s_groups = malloc(n_groups * sizeof(char*));

		// Convert groups to a string format
		for(int i = 0; i < n_groups; i++) {
			size_t length = groups[i].rm_eo - groups[i].rm_so + 1;
			char* match = malloc(length * sizeof(char));
			memcpy(match, line + groups[i].rm_so, length);
			match[length - 1] = '\0';
			s_groups[i] = match;
		}

		// Transfer groups to passed in variables
		strcpy(pre, s_groups[1]);
		s_header = s_groups[2];
		strcpy(value, s_groups[3]);
		strcpy(post, s_groups[4]);

		// Free memory
		free(s_groups[0]);
		free(s_groups[1]);
		// s_groups[2] is still saved in s_header, which is freed later
		free(s_groups[3]);
		free(s_groups[4]);
		free(s_groups);

		// Set header based on s_header
		if(strcasecmp(s_header, "theme") == 0)
			*header = theme;
		else if(strcasecmp(s_header, "color") == 0)
			*header = color;
		else
			*header = unknown;

		free(s_header);
	}

	free(groups);
	return ret == 0;
}

static bool parse_config(const char* input_filename, const char* output_filename, const char* target_theme) {
	FILE *in_file, *out_file;
	char buff[MAX_LINE_LENGTH];
	char section_theme[MAX_LINE_LENGTH] = "";
	int line = 0;

	in_file = fopen(input_filename, "r");
	out_file = fopen(output_filename, "w");

	while(fgets(buff, MAX_LINE_LENGTH, in_file)) {
		line++;

		char pre[MAX_LINE_LENGTH], post[MAX_LINE_LENGTH];
		enum header_type *header = malloc(sizeof(enum header_type));
		char val[MAX_HEADER_VALUE_LENGTH];
		
		bool header_found = parse_line(buff, pre, header, val, post);

		if(header_found) {
			switch(*header) {
				case theme:
					if(strcmp(val, THEME_END_VALUE) == 0) {
						strcpy(section_theme, "");
						logger(trace, "%d: Ending section\n", line);
					} else {
						strcpy(section_theme, val);
						logger(trace, "%d: New section: %s\n", line, section_theme);
					}

					strcpy(buff, "");

					break;
				case color:
					logger(trace, "Color: %s\n", val); // TODO: get color from val

					strcpy(buff, pre);
					strcat(buff, val);
					strcat(buff, post);
					break;
				default:
					logger(warning, "Unknown header in '%s' on line: %d\n", input_filename, line);
			}
		}

		if(strcmp(section_theme, "") == 0 || strcasecmp(section_theme, target_theme) == 0) {
			if(strcmp(buff, "") != 0)
				logger(trace, "%d: writing line: %s", line, buff);

			fprintf(out_file, "%s", buff);
		}

		free(header);
	}

	fclose(in_file);
	fclose(out_file);

	logger(info, "Wrote '%s' to '%s'\n", input_filename, output_filename);

	return true;
}

static bool parse_colors() {
	return true;
}

bool parser(const struct arguments *args) {
	HOME = getenv("HOME");
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
		in_path = malloc(sizeof(char) * (strlen(HOME) + strlen(SRC_DIR) + 1));
		strcpy(in_path, HOME);
		strcat(in_path, SRC_DIR);
	} else
		in_path = args->input_dirname;

	if(!args->output_dirname) {
		out_path = malloc(sizeof(char) * (strlen(HOME) + strlen(SRC_DIR) + 1));
		strcpy(out_path, HOME);
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
			char input_filename[MAX_PATH_LENGTH], output_filename[MAX_PATH_LENGTH];
			getfullpath(input_filename, in_path, entry->d_name);
			getfullpath(output_filename, out_path, entry->d_name);

			logger(debug, "Parsing: %s\n", input_filename);
			if(!parse_config(input_filename, output_filename, args->theme))
				return false;
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
