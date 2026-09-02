#ifndef TRACE_H
#define TRACE_H
#include <stdarg.h>
/*
    Copyright (C) potato-master369 2026-
    Licensed under the MIT license.

    Copyright (C) potato-master369 2026-
    Modified for desktop-gremlin-linux (MIT)

    Feel free to do as you wish with this library.
*/
#define TRACE 0
#define DEBUG 1
#define INFO 2
#define WARN 3
#define ERROR 4
#define FATAL 5

void _raw_log (char level, const char *file, int line, const char *format, ...);
void _raw_vlog (char level, const char *file, int line, const char *format, va_list v);
void _raw_flog (const char *toFile, char level, const char *file, int line, const char *format, ...);
void trace_set_loglevel (char level);

/*
    Function provided by trace.c to log an item. Arguments:
    * char level - loglevel (see trace.h for macros)
    * const char *format - string to print
    (int line and const char *file are handled by the macro
    in trace.h)
*/
#define trace_log(level, format, ...) _raw_log (level, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define trace_flog(file, level, format, ...) _raw_flog (file, level, __FILE__, __LINE__, format, ##__VA_ARGS__)
#endif
