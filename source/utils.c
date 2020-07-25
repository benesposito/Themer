#include "utils.h"

static void vjoin(char* str, char* delim, size_t argc, va_list valist) {
	strcpy(str, "");

	for(size_t i = 0; i < argc; i++) {
		strcat(str, va_arg(valist, char*));
		strcat(str, delim);
	}

	str[strlen(str) - 1] = '\0';
}

void join(char* str, char* delim, size_t argc, ...) {
	va_list valist;
	va_start(valist, argc);
	vjoin(str, delim, argc, valist);
	va_end(valist);
}

void getpath(char* path, size_t argc, ...) {
	va_list valist;
	va_start(valist, argc);
	vjoin(path, "/", argc, valist);
	va_end(valist);
}
