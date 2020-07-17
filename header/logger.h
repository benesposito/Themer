#pragma once

enum severity_level {
	trace = 0,
	debug = 1,
	info = 2,
	warning = 3,
	error = 4
};

void set_logger_severity_level(enum severity_level);
void logger(enum severity_level severity, char* format, ...);
