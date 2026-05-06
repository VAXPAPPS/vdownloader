#include "aria2_types.h"
#include <math.h>
#include <string.h>

DownloadItem *
download_item_new (void)
{
    DownloadItem *item = g_new0 (DownloadItem, 1);
    item->state = DOWNLOAD_STATE_WAITING;
    return item;
}

void
download_item_free (DownloadItem *item)
{
    if (!item) return;
    g_free (item->gid);
    g_free (item->name);
    g_free (item->url);
    g_free (item->dir);
    g_free (item->mime_type);
    g_free (item->error_message);
    g_free (item);
}

DownloadItem *
download_item_copy (const DownloadItem *src)
{
    if (!src) return NULL;
    DownloadItem *dst = g_new0 (DownloadItem, 1);
    *dst = *src;
    dst->gid           = g_strdup (src->gid);
    dst->name          = g_strdup (src->name);
    dst->url           = g_strdup (src->url);
    dst->dir           = g_strdup (src->dir);
    dst->mime_type     = g_strdup (src->mime_type);
    dst->error_message = g_strdup (src->error_message);
    return dst;
}

static DownloadState
parse_state (const gchar *s)
{
    if (!s) return DOWNLOAD_STATE_WAITING;
    if (g_strcmp0 (s, "active")   == 0) return DOWNLOAD_STATE_ACTIVE;
    if (g_strcmp0 (s, "waiting")  == 0) return DOWNLOAD_STATE_WAITING;
    if (g_strcmp0 (s, "paused")   == 0) return DOWNLOAD_STATE_PAUSED;
    if (g_strcmp0 (s, "complete") == 0) return DOWNLOAD_STATE_COMPLETE;
    if (g_strcmp0 (s, "error")    == 0) return DOWNLOAD_STATE_ERROR;
    if (g_strcmp0 (s, "removed")  == 0) return DOWNLOAD_STATE_REMOVED;
    return DOWNLOAD_STATE_WAITING;
}

DownloadItem *
download_item_from_json (JsonObject *obj)
{
    if (!obj) return NULL;
    DownloadItem *item = download_item_new ();

    item->gid   = g_strdup (json_object_get_string_member_with_default (obj, "gid", ""));
    item->state = parse_state (json_object_get_string_member_with_default (obj, "status", "waiting"));
    item->total_length     = g_ascii_strtoll (
        json_object_get_string_member_with_default (obj, "totalLength", "0"), NULL, 10);
    item->completed_length = g_ascii_strtoll (
        json_object_get_string_member_with_default (obj, "completedLength", "0"), NULL, 10);
    item->download_speed   = g_ascii_strtoll (
        json_object_get_string_member_with_default (obj, "downloadSpeed", "0"), NULL, 10);
    item->upload_speed     = g_ascii_strtoll (
        json_object_get_string_member_with_default (obj, "uploadSpeed", "0"), NULL, 10);
    item->dir              = g_strdup (
        json_object_get_string_member_with_default (obj, "dir", ""));
    item->num_pieces       = (gint)json_object_get_int_member_with_default (obj, "numPieces", 0);
    item->piece_length     = (gint)json_object_get_int_member_with_default (obj, "pieceLength", 0);

    if (item->total_length > 0)
        item->progress = (gdouble)item->completed_length / (gdouble)item->total_length;

    /* Get first file name from "files" array */
    if (json_object_has_member (obj, "files")) {
        JsonArray *files = json_object_get_array_member (obj, "files");
        if (json_array_get_length (files) > 0) {
            JsonObject *file0 = json_array_get_object_element (files, 0);
            const gchar *path = json_object_get_string_member_with_default (file0, "path", "");
            if (path && *path)
                item->name = g_path_get_basename (path);
            /* Get first URI */
            if (json_object_has_member (file0, "uris")) {
                JsonArray *uris = json_object_get_array_member (file0, "uris");
                if (json_array_get_length (uris) > 0) {
                    JsonObject *uri0 = json_array_get_object_element (uris, 0);
                    item->url = g_strdup (
                        json_object_get_string_member_with_default (uri0, "uri", ""));
                    if (!item->name || g_strcmp0 (item->name, "") == 0) {
                        g_free (item->name);
                        item->name = g_path_get_basename (item->url);
                    }
                }
            }
        }
    }

    if (!item->name || g_strcmp0 (item->name, "") == 0) {
        g_free (item->name);
        item->name = g_strdup (item->gid);
    }

    /* Error message */
    if (item->state == DOWNLOAD_STATE_ERROR && json_object_has_member (obj, "errorMessage"))
        item->error_message = g_strdup (json_object_get_string_member (obj, "errorMessage"));

    return item;
}

gchar *
download_item_format_size (gint64 bytes)
{
    if (bytes < 0)           return g_strdup ("unknown");
    if (bytes < 1024)        return g_strdup_printf ("%" G_GINT64_FORMAT " B",  bytes);
    if (bytes < 1024*1024)   return g_strdup_printf ("%.1f KB", bytes / 1024.0);
    if (bytes < 1024*1024*1024LL) return g_strdup_printf ("%.1f MB", bytes / (1024.0*1024.0));
    return g_strdup_printf ("%.2f GB", bytes / (1024.0*1024.0*1024.0));
}

gchar *
download_item_format_speed (gint64 bps)
{
    if (bps <= 0) return g_strdup ("0 KB/s");
    if (bps < 1024)      return g_strdup_printf ("%" G_GINT64_FORMAT " B/s",  bps);
    if (bps < 1024*1024) return g_strdup_printf ("%.1f KB/s", bps / 1024.0);
    return g_strdup_printf ("%.2f MB/s", bps / (1024.0*1024.0));
}

const gchar *
download_item_state_name (DownloadState state)
{
    switch (state) {
    case DOWNLOAD_STATE_ACTIVE:   return "active";
    case DOWNLOAD_STATE_WAITING:  return "waiting";
    case DOWNLOAD_STATE_PAUSED:   return "paused";
    case DOWNLOAD_STATE_COMPLETE: return "complete";
    case DOWNLOAD_STATE_ERROR:    return "error";
    case DOWNLOAD_STATE_REMOVED:  return "removed";
    default:                      return "unknown";
    }
}
