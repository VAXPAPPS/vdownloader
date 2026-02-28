/*
 * VXAPDownloader - Logger
 */

#ifndef VDL_LOGGER_H
#define VDL_LOGGER_H

#include <glib.h>

void vdl_logger_init     (void);
void vdl_logger_shutdown (void);
void vdl_logger_info     (const char *format, ...) G_GNUC_PRINTF(1, 2);
void vdl_logger_warn     (const char *format, ...) G_GNUC_PRINTF(1, 2);
void vdl_logger_error    (const char *format, ...) G_GNUC_PRINTF(1, 2);
void vdl_logger_debug    (const char *format, ...) G_GNUC_PRINTF(1, 2);

#endif /* VDL_LOGGER_H */
