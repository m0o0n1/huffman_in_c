#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdio.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void print_error(char *);
void print_success(char *);

#endif
