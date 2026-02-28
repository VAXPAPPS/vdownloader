/*
 * VXAPDownloader - Utility Functions Implementation
 */

#include "vdl-utils.h"
#include "vdl-enums.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

char *
vdl_utils_format_size (int64_t bytes)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double size = (double) bytes;
    int unit = 0;

    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        unit++;
    }

    if (unit == 0)
        return g_strdup_printf ("%d %s", (int)size, units[unit]);
    else
        return g_strdup_printf ("%.1f %s", size, units[unit]);
}

char *
vdl_utils_format_speed (double bytes_per_sec)
{
    if (bytes_per_sec <= 0)
        return g_strdup ("0 B/s");

    const char *units[] = {"B/s", "KB/s", "MB/s", "GB/s"};
    double speed = bytes_per_sec;
    int unit = 0;

    while (speed >= 1024.0 && unit < 3) {
        speed /= 1024.0;
        unit++;
    }

    return g_strdup_printf ("%.1f %s", speed, units[unit]);
}

char *
vdl_utils_format_time (int seconds)
{
    if (seconds < 0)
        return g_strdup ("∞");
    if (seconds == 0)
        return g_strdup ("0s");

    int hours = seconds / 3600;
    int mins  = (seconds % 3600) / 60;
    int secs  = seconds % 60;

    if (hours > 0)
        return g_strdup_printf ("%dh %dm %ds", hours, mins, secs);
    else if (mins > 0)
        return g_strdup_printf ("%dm %ds", mins, secs);
    else
        return g_strdup_printf ("%ds", secs);
}

char *
vdl_utils_filename_from_url (const char *url)
{
    if (!url) return g_strdup ("unknown");

    /* Find the last '/' before any '?' */
    const char *query = strchr (url, '?');
    const char *last_slash = NULL;
    const char *end = query ? query : url + strlen(url);

    for (const char *p = url; p < end; p++) {
        if (*p == '/') last_slash = p;
    }

    if (last_slash && last_slash + 1 < end) {
        size_t len = (size_t)(end - last_slash - 1);
        char *name = g_strndup (last_slash + 1, len);

        /* URL decode */
        g_autofree char *decoded = g_uri_unescape_string (name, NULL);
        g_free (name);
        if (decoded)
            return g_strdup (decoded);
    }

    return g_strdup ("download");
}

const char *
vdl_utils_get_extension (const char *filename)
{
    if (!filename) return "";
    const char *dot = strrchr (filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

int
vdl_utils_category_from_extension (const char *ext)
{
    if (!ext || *ext == '\0') return VDL_CATEGORY_GENERAL;

    /* Video */
    if (g_ascii_strcasecmp(ext, "mp4") == 0 || g_ascii_strcasecmp(ext, "mkv") == 0 ||
        g_ascii_strcasecmp(ext, "avi") == 0 || g_ascii_strcasecmp(ext, "mov") == 0 ||
        g_ascii_strcasecmp(ext, "wmv") == 0 || g_ascii_strcasecmp(ext, "flv") == 0 ||
        g_ascii_strcasecmp(ext, "webm") == 0)
        return VDL_CATEGORY_VIDEO;

    /* Music */
    if (g_ascii_strcasecmp(ext, "mp3") == 0 || g_ascii_strcasecmp(ext, "flac") == 0 ||
        g_ascii_strcasecmp(ext, "aac") == 0 || g_ascii_strcasecmp(ext, "ogg") == 0 ||
        g_ascii_strcasecmp(ext, "wav") == 0 || g_ascii_strcasecmp(ext, "wma") == 0)
        return VDL_CATEGORY_MUSIC;

    /* Documents */
    if (g_ascii_strcasecmp(ext, "pdf") == 0 || g_ascii_strcasecmp(ext, "doc") == 0 ||
        g_ascii_strcasecmp(ext, "docx") == 0 || g_ascii_strcasecmp(ext, "xls") == 0 ||
        g_ascii_strcasecmp(ext, "xlsx") == 0 || g_ascii_strcasecmp(ext, "ppt") == 0 ||
        g_ascii_strcasecmp(ext, "txt") == 0)
        return VDL_CATEGORY_DOCUMENTS;

    /* Programs */
    if (g_ascii_strcasecmp(ext, "deb") == 0 || g_ascii_strcasecmp(ext, "rpm") == 0 ||
        g_ascii_strcasecmp(ext, "AppImage") == 0 || g_ascii_strcasecmp(ext, "exe") == 0 ||
        g_ascii_strcasecmp(ext, "msi") == 0 || g_ascii_strcasecmp(ext, "sh") == 0)
        return VDL_CATEGORY_PROGRAMS;

    /* Compressed */
    if (g_ascii_strcasecmp(ext, "zip") == 0 || g_ascii_strcasecmp(ext, "rar") == 0 ||
        g_ascii_strcasecmp(ext, "7z") == 0 || g_ascii_strcasecmp(ext, "tar") == 0 ||
        g_ascii_strcasecmp(ext, "gz") == 0 || g_ascii_strcasecmp(ext, "xz") == 0)
        return VDL_CATEGORY_COMPRESSED;

    /* Torrent */
    if (g_ascii_strcasecmp(ext, "torrent") == 0)
        return VDL_CATEGORY_TORRENT;

    return VDL_CATEGORY_GENERAL;
}

char *
vdl_utils_generate_id (void)
{
    return g_uuid_string_random ();
}

gboolean
vdl_utils_is_valid_url (const char *url)
{
    if (!url) return FALSE;
    return g_str_has_prefix(url, "http://")  ||
           g_str_has_prefix(url, "https://") ||
           g_str_has_prefix(url, "ftp://")   ||
           g_str_has_prefix(url, "magnet:");
}

gboolean
vdl_utils_is_magnet_link (const char *url)
{
    return url && g_str_has_prefix(url, "magnet:");
}

gboolean
vdl_utils_is_torrent_file (const char *path)
{
    if (!path) return FALSE;
    return g_str_has_suffix(path, ".torrent");
}
