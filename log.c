#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void log(LogLevel level, char *format, ...) {
    va_list vl;
    va_start(vl, format);
    printf("")
    int ret = vprintf(format, vl);


}

void debug(char *format, ...) {}

void info(char *format, ...) {}

void warning(char *format, ...) {}

void error(char *format, ...) {}

void fatal(char *format, ...) {}
