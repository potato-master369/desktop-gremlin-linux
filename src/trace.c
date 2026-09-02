#include "trace.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/*
    Copyright (C) potato-master369 2026-
    Licensed under the MIT license.

    Feel free to do as you wish with this library.
*/

/*
    TRACE_FORMAT - macro containing the format used for logging. For a simple guide,
    modify %-12s to a number of your liking or length needed for filenames, e.g.
    %-20s, etc. Repeat similarly for %-4d, for line numbers.
*/
#ifndef TRACE_FORMAT
#define TRACE_FORMAT "\x1b[4;39m%s\x1b[0m %s\x1b[0m \x1b[0;90m%-12s:%-4d\x1b[0m "
#endif
int loglevel = -1;

/*
    Function provided by trace.c to log an item. Arguments:
    * char level - loglevel (see trace.h for macros)
    * const char *format - string to print
    (int line and const char *file are handled by the macro
    in trace.h)
*/
void
_raw_log (char level, const char *file, int line, const char *format, ...)
{
    if (level > loglevel)
    {
        va_list args;
        va_start (args, format);
        time_t rawtime;
        struct tm *timeinfo;

        time (&rawtime);                 // Get the current time as time_t
        timeinfo = localtime (&rawtime); // Convert to local time structure

        char time[20];
        strftime (time, sizeof (time), "%H:%M:%S", timeinfo);
        fprintf (level > INFO ? stderr : stdout, TRACE_FORMAT, time, (level == TRACE ? "\x1b[0;94m[TRACE]" : level == DEBUG ? "\x1b[0;36m[DEBUG]"
                                                                                                         : level == INFO    ? "\x1b[0;32m[ INFO]"
                                                                                                         : level == WARN    ? "\x1b[0;93m[ WARN]"
                                                                                                         : level == ERROR   ? "\x1b[0;91m[ERROR]"
                                                                                                         : level == FATAL   ? "\x1b[0;95m[FATAL]"
                                                                                                                            : "\x1b[0;94m[UNKWN]"),
                 file, line);
        vfprintf (level > INFO ? stderr : stdout, format, args);
        va_end (args);
    }
    return;
}

void
_raw_vlog (char level, const char *file, int line, const char *format, va_list v)
{
    if (level > loglevel)
    {
        time_t rawtime;
        struct tm *timeinfo;

        time (&rawtime);                 // Get the current time as time_t
        timeinfo = localtime (&rawtime); // Convert to local time structure

        char time[20];
        strftime (time, sizeof (time), "%H:%M:%S", timeinfo);
        fprintf (level > INFO ? stderr : stdout, TRACE_FORMAT, time, (level == TRACE ? "\x1b[0;94m[TRACE]" : level == DEBUG ? "\x1b[0;36m[DEBUG]"
                                                                                                         : level == INFO    ? "\x1b[0;32m[ INFO]"
                                                                                                         : level == WARN    ? "\x1b[0;93m[ WARN]"
                                                                                                         : level == ERROR   ? "\x1b[0;91m[ERROR]"
                                                                                                         : level == FATAL   ? "\x1b[0;95m[FATAL]"
                                                                                                                            : "\x1b[0;94m[UNKWN]"),
                 file, line);
        vfprintf (level > INFO ? stderr : stdout, format, v);
    }
    return;
}

/*
    Function similar to raw_log, but to log to a file as per const char *toFile.
    See raw_log for explanations of other fields.
*/
void
_raw_flog (const char *toFile, char level, const char *file, int line, const char *format, ...)
{
    if (level > loglevel)
    {
        FILE *file_ptr = fopen (toFile, "a");
        if (file_ptr == NULL)
        {
            _raw_log (FATAL, __FILE__, __LINE__, "Error opening file!\n");
            exit (EXIT_FAILURE);
        }
        va_list args;
        va_start (args, format);
        time_t rawtime;
        struct tm *timeinfo;

        time (&rawtime);                 // Get the current time as time_t
        timeinfo = localtime (&rawtime); // Convert to local time structure

        char time[20];
        strftime (time, sizeof (time), "%H:%M:%S", timeinfo);
        fprintf (file_ptr, TRACE_FORMAT, time, (level == TRACE ? "\x1b[0;94m[TRACE]" : level == DEBUG ? "\x1b[0;36m[DEBUG]"
                                                                                   : level == INFO    ? "\x1b[0;32m[ INFO]"
                                                                                   : level == WARN    ? "\x1b[0;93m[ WARN]"
                                                                                   : level == ERROR   ? "\x1b[0;91m[ERROR]"
                                                                                   : level == FATAL   ? "\x1b[0;95m[FATAL]"
                                                                                                      : "\x1b[0;94m[UNKWN]"),
                 file, line);
        vfprintf (file_ptr, format, args);
        va_end (args);
    }
    return;
}

/*
    Function to change loglevel. Uses top-level declaration of int loglevel here.
*/
void
trace_set_loglevel (char level)
{
    _raw_log (TRACE, __FILE__, __LINE__, "Setting loglevel to: %d\n", level);
    loglevel = level;
    return;
}
