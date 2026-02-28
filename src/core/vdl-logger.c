/*
 * VXAPDownloader - Logger Implementation
 */

#include "vdl-logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static FILE *log_file = NULL;

static void
vdl_logger_write (const char *level, const char *format, va_list args)
{
    time_t now = time (NULL);
    struct tm *tm = localtime (&now);
    char time_buf[64];
    strftime (time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm);

    char message[4096];
    vsnprintf (message, sizeof(message), format, args);

    g_print ("[%s] [%s] %s\n", time_buf, level, message);

    if (log_file) {
        fprintf (log_file, "[%s] [%s] %s\n", time_buf, level, message);
        fflush (log_file);
    }
}

void
vdl_logger_init (void)
{
    g_autofree char *log_dir = g_build_filename (
        g_get_user_data_dir (), "vxap-downloader", "logs", NULL
    );
    g_mkdir_with_parents (log_dir, 0755);

    g_autofree char *log_path = g_build_filename (log_dir, "vxap-downloader.log", NULL);
    log_file = fopen (log_path, "a");
}

void
vdl_logger_shutdown (void)
{
    if (log_file) {
        fclose (log_file);
        log_file = NULL;
    }
}

void vdl_logger_info (const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vdl_logger_write ("INFO", format, args);
    va_end (args);
}

void vdl_logger_warn (const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vdl_logger_write ("WARN", format, args);
    va_end (args);
}

void vdl_logger_error (const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vdl_logger_write ("ERROR", format, args);
    va_end (args);
}

void vdl_logger_debug (const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vdl_logger_write ("DEBUG", format, args);
    va_end (args);
}
