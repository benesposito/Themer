#include "parser.h"
#include "logger.h"
#include "utils.h"

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

#define MAX_COLOR_DEFS 50
#define MAX_COLOR_NAME_LENGTH 30
#define COLOR_CODE_LENGTH 8

#define THEMER_DIR ".config/themer"
#define SRC_DIR "src"
#define BIN_DIR "bin"

#define THEME_END_VALUE "ENDTHEME"

#define COLORS_PATH 

struct color_entry {
	char name[MAX_COLOR_NAME_LENGTH];
	char hex[COLOR_CODE_LENGTH];
};

enum header_type { theme, color, unknown };

char* HOME_DIR;
struct color_entry colors[MAX_COLOR_DEFS];
int n_colors;

static int parse_colors(struct color_entry *colors, char* s_theme) { // returns -1 for failure, otherwise returns n_colors
	char colors_file_path[MAX_PATH_LENGTH];
	FILE* colors_file;
	char buff[MAX_LINE_LENGTH];
	bool found_target_header = false;

	getpath(colors_file_path, 3, HOME_DIR, THEMER_DIR, "colors.ini");
 	colors_file = fopen(colors_file_path, "r");
	logger(debug, "[PARSING COLORS] (%s)\n", colors_file_path);

	if(!colors_file) {
		logger(error, "colors.ini could not be found!\n");
		return -1;
	}

	size_t n_colors = 0;

	while(fgets(buff, MAX_LINE_LENGTH, colors_file)) {
		char section_name[MAX_COLOR_NAME_LENGTH + 2];
		int ret = sscanf(buff, "[%[^]]]", section_name);

		if(ret == 1) {
			logger(debug, "Found color header: [%s]\n", section_name);

			if(strcasecmp(section_name, s_theme) == 0) {
				if(!found_target_header)
					found_target_header = true;
				else {
					logger(warning, "[%s] was defined twice in colors.ini! Using second definition", theme);
				}
			} else
				found_target_header = false;
		}

		if(found_target_header) {
			char key[MAX_COLOR_NAME_LENGTH + 1];
			char value[COLOR_CODE_LENGTH + 1];

			int ret = sscanf(buff, "%[^=]=%[^\n]\n", key, value);
			if(ret == 2) {
				logger(debug, "%s=%s\n", key, value);
				strcpy(colors[n_colors].name, key);
				strcpy(colors[n_colors].hex, value);
				n_colors++;
			}
		}
	}

	logger(debug, LOGGER_SECTION_FOOTER);

	return n_colors;
}

static bool parse_line(const char* line, char* pre, enum header_type *header, char* value, char* post) {
	char* s_header;

	// Get matching groups from 'line'
	regex_t regex;
	regcomp(&regex, "(.*?)\\{(\\w*):([A-Za-z-]*)\\}(.*)", REG_EXTENDED);
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
	} /* else {
		logger(error, "Issue with regex matching\n");
		int n_errlen = 50;
		char* s_err = malloc(n_errlen * sizeof(char));
		regerror(ret, &regex, s_err, n_errlen);
		logger(error, "%s\n", s_err);
	} */

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

	bool success = true;

	while(fgets(buff, MAX_LINE_LENGTH, in_file) && success) {
		line++;

		char pre[MAX_LINE_LENGTH], post[MAX_LINE_LENGTH];
		enum header_type *header = malloc(sizeof(enum header_type));
		char val[MAX_HEADER_VALUE_LENGTH];
		
		while(parse_line(buff, pre, header, val, post)) {
			switch(*header) {
				case theme:
					if(strcmp(val, THEME_END_VALUE) == 0) {
						strcpy(section_theme, "");
						logger(debug, "%d: Ending section\n", line);
					} else {
						strcpy(section_theme, val);
						logger(debug, "%d: New section: %s\n", line, section_theme);
					}

					strcpy(buff, "");

					break;
				case color:
					; // apparently this ';' is needed because the first line of a case can't be a variable declaration in C (??)
					char hex[COLOR_CODE_LENGTH];
					bool b_found_color = false;

					for(size_t i = 0; i < n_colors; i++) {
						if(strcmp(val, colors[i].name) == 0) {
							strcpy(hex, colors[i].hex);
							b_found_color = true;
							break;
						}
					}

					if(b_found_color) {
						logger(debug, "%d: Color: %s\n", line, hex);

						strcpy(buff, pre);
						strcat(buff, hex);
						strcat(buff, post);
					} else {
						logger(error, "Undefined color '%s' in '%s'\n", val, input_filename);
						success = false;
					}

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

	if(success)
		logger(info, "Wrote '%s' to '%s'\n", input_filename, output_filename);
	else
		remove(output_filename);

	return true;
}

bool parser(const struct arguments *args) {
	HOME_DIR = getenv("HOME");
	char *in_path, *out_path;

	if(args->input_dirname)
		in_path = args->input_dirname;
	else {
		in_path = malloc(sizeof(char) * (strlen(HOME_DIR) + strlen(THEMER_DIR) + strlen(SRC_DIR) + 1));
		getpath(in_path, 3, HOME_DIR, THEMER_DIR, SRC_DIR);
	}

	if(args->output_dirname)
		out_path = args->output_dirname;
	else {
		out_path = malloc(sizeof(char) * (strlen(HOME_DIR) + strlen(THEMER_DIR) + strlen(SRC_DIR) + 1));
		getpath(out_path, 3, HOME_DIR, THEMER_DIR, BIN_DIR);
	}

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

	n_colors = parse_colors(colors, args->theme);

	if(args->input_filename) {
		char* output_filename = malloc(sizeof(char) * (strlen(args->input_filename) + strlen(args->theme) + 2));
		strcpy(output_filename, args->input_filename);
		strcat(output_filename, ".");
		strcat(output_filename, args->theme);

		bool ret = parse_config(args->input_filename, output_filename, args->theme);

		free(output_filename);
		return ret;
	}

	struct dirent *entry;
	bool success = true;

	while((entry = readdir(in_dir)) != NULL) {
		if(entry->d_type == DT_REG) {
			char input_filename[MAX_PATH_LENGTH], output_filename[MAX_PATH_LENGTH];
			getpath(input_filename, 2, in_path, entry->d_name);
			getpath(output_filename, 2, out_path, entry->d_name);

			logger(debug, "Parsing: %s\n", input_filename);
			if(!parse_config(input_filename, output_filename, args->theme)) {
				success = false;
				break;
			}
		}
	}

	if(!args->input_dirname) {
		free(in_path);
		free(out_path);
	}

	closedir(in_dir);
	closedir(out_dir);

	return success;
}
