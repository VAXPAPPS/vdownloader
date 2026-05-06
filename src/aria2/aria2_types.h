#pragma once

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

/* Download states mirroring aria2 status */
typedef enum {
    DOWNLOAD_STATE_ACTIVE,
    DOWNLOAD_STATE_WAITING,
    DOWNLOAD_STATE_PAUSED,
    DOWNLOAD_STATE_COMPLETE,
    DOWNLOAD_STATE_ERROR,
    DOWNLOAD_STATE_REMOVED,
} DownloadState;

/* A download item */
typedef struct {
    gchar        *gid;           /* aria2 global ID */
    gchar        *name;          /* filename / display name */
    gchar        *url;           /* original URL */
    gchar        *dir;           /* download directory */
    gchar        *mime_type;     /* MIME type if available */
    DownloadState state;
    gint64        total_length;  /* bytes */
    gint64        completed_length;
    gint64        download_speed; /* bytes/sec */
    gint64        upload_speed;
    gint          num_connections;
    gint          num_pieces;
    gint          piece_length;
    gdouble       progress;      /* 0.0 – 1.0 */
    gchar        *error_message;
    gint64        created_at;    /* unix timestamp */
    gint64        completed_at;
} DownloadItem;

DownloadItem *download_item_new  (void);
DownloadItem *download_item_copy (const DownloadItem *item);
void          download_item_free (DownloadItem *item);

/* Parse a JsonObject (aria2 tellStatus result) into a DownloadItem */
DownloadItem *download_item_from_json (JsonObject *obj);

/* Human-readable helpers */
gchar *download_item_format_size   (gint64 bytes);
gchar *download_item_format_speed  (gint64 bytes_per_sec);
const gchar *download_item_state_name (DownloadState state);
