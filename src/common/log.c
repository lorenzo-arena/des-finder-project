#include <stdio.h>
#include <stdarg.h>

#include "log.h"

void log_error(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    fprintf(stdout, "ERROR: ");
    vfprintf(stdout, format, ap);
    fprintf(stdout, "\n");
    va_end(ap);
}

void log_info(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    fprintf(stdout, "INFO: ");
    vfprintf(stdout, format, ap);
    fprintf(stdout, "\n");
    va_end(ap);
}
