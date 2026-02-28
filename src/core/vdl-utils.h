/*
 * VXAPDownloader - Utility Functions
 */

#ifndef VDL_UTILS_H
#define VDL_UTILS_H

#include <glib.h>
#include <stdint.h>

/* Format bytes to human-readable string (e.g. 1.5 GB) */
char *vdl_utils_format_size (int64_t bytes);

/* Format speed to human-readable string (e.g. 2.3 MB/s) */
char *vdl_utils_format_speed (double bytes_per_sec);

/* Format time to human-readable string (e.g. 2h 15m) */
char *vdl_utils_format_time (int seconds);

/* Extract filename from URL */
char *vdl_utils_filename_from_url (const char *url);

/* Get file extension from filename */
const char *vdl_utils_get_extension (const char *filename);

/* Determine category from file extension */
int vdl_utils_category_from_extension (const char *ext);

/* Generate unique ID */
char *vdl_utils_generate_id (void);

/* Check if a URL is valid */
gboolean vdl_utils_is_valid_url (const char *url);

/* Check if a string is a magnet link */
gboolean vdl_utils_is_magnet_link (const char *url);

/* Check if a string is a torrent file path */
gboolean vdl_utils_is_torrent_file (const char *path);

#endif /* VDL_UTILS_H */
