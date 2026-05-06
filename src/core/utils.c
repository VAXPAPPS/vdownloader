#include "core/utils.h"
#include <glib.h>
#include <string.h>

gchar *
utils_format_size (gint64 bytes)
{
    if (bytes < 0)                   return g_strdup ("—");
    if (bytes == 0)                  return g_strdup ("0 B");
    if (bytes < 1024)                return g_strdup_printf ("%" G_GINT64_FORMAT " B", bytes);
    if (bytes < 1024 * 1024)         return g_strdup_printf ("%.1f KB", bytes / 1024.0);
    if (bytes < 1024 * 1024 * 1024LL)return g_strdup_printf ("%.1f MB", bytes / (1024.0*1024));
    return g_strdup_printf ("%.2f GB", bytes / (1024.0*1024*1024));
}

gchar *
utils_format_speed (gint64 bps)
{
    if (bps <= 0)          return g_strdup ("0 KB/s");
    if (bps < 1024)        return g_strdup_printf ("%" G_GINT64_FORMAT " B/s", bps);
    if (bps < 1024*1024)   return g_strdup_printf ("%.1f KB/s", bps / 1024.0);
    return g_strdup_printf ("%.2f MB/s", bps / (1024.0*1024));
}

gchar *
utils_format_eta (gint64 remaining, gint64 speed)
{
    if (speed <= 0 || remaining <= 0) return g_strdup ("—");
    gint64 secs = remaining / speed;
    if (secs < 60)   return g_strdup_printf ("%" G_GINT64_FORMAT " s",  secs);
    if (secs < 3600) return g_strdup_printf ("%" G_GINT64_FORMAT " m %02" G_GINT64_FORMAT " s",
                                              secs/60, secs%60);
    return g_strdup_printf ("%" G_GINT64_FORMAT " h %02" G_GINT64_FORMAT " m", secs/3600, (secs%3600)/60);
}

gboolean
utils_is_valid_url (const gchar *url)
{
    if (!url || !*url) return FALSE;
    return g_str_has_prefix (url, "http://")  ||
           g_str_has_prefix (url, "https://") ||
           g_str_has_prefix (url, "ftp://")   ||
           g_str_has_prefix (url, "ftps://")  ||
           g_str_has_prefix (url, "magnet:")  ||
           g_str_has_suffix (url, ".torrent");
}

gchar *
utils_basename_from_url (const gchar *url)
{
    if (!url || !*url) return g_strdup ("download");
    const gchar *q = strchr (url, '?');
    gchar *clean = q ? g_strndup (url, q - url) : g_strdup (url);
    gchar *base = g_path_get_basename (clean);
    g_free (clean);
    return base;
}

gchar *
utils_get_default_download_dir (void)
{
    const gchar *dir = g_get_user_special_dir (G_USER_DIRECTORY_DOWNLOAD);
    return g_strdup (dir ? dir : g_get_home_dir ());
}

gchar *
utils_get_host_from_url (const gchar *url)
{
    if (!url) return NULL;
    const gchar *start = strstr (url, "://");
    if (!start) start = url; else start += 3;
    const gchar *end = strchr (start, '/');
    if (!end) end = url + strlen(url);
    
    gchar *host = g_strndup (start, end - start);
    const gchar *scheme = g_str_has_prefix(url, "https") ? "https" : "http";
    gchar *ref = g_strdup_printf ("%s://%s/", scheme, host);
    g_free (host);
    return ref;
}
