#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

static int m_severity = error;

void set_logger_severity_level(enum severity_level severity) {
	m_severity = severity;
}

void logger(enum severity_level severity, char* format, ...) {
	if(severity >= m_severity) {
		va_list args;
		va_start(args, format);

		char* prepend;

		switch(severity) {
			case trace:
				prepend = "trace: ";
				break;
			case debug:
				prepend = "debug: ";
				break;
			case warning:
				prepend = "warning: ";
				break;
			case error:
				prepend = "error: ";
				break;
			default:
				prepend = "";
		}

		char* full_format = malloc(sizeof(char) * (strlen(prepend) + strlen(format) + 1));
		strcpy(full_format, prepend);
		strcat(full_format, format);

		vfprintf(stdout, full_format, args);

		free(full_format);
		va_end(args);
	}
}
