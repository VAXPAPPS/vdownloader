#pragma once

#include <glib.h>
#include <json-glib/json-glib.h>
#include "aria2_types.h"

#define ARIA2_DEFAULT_HOST   "http://localhost"
#define ARIA2_DEFAULT_PORT   6800
#define ARIA2_RPC_PATH       "/jsonrpc"

typedef struct _Aria2Client Aria2Client;

typedef void (*Aria2Callback)      (Aria2Client *client, gpointer result, GError *error, gpointer user_data);
typedef void (*Aria2StatusCallback)(Aria2Client *client, DownloadItem *item, GError *error, gpointer user_data);
typedef void (*Aria2ListCallback)  (Aria2Client *client, GList *items, GError *error, gpointer user_data);
typedef void (*Aria2StatsCallback) (Aria2Client *client, gint64 dl_speed, gint64 ul_speed,
                                    gint num_active, gint num_waiting, gint num_stopped,
                                    GError *error, gpointer user_data);

/* Lifecycle */
Aria2Client *aria2_client_new       (const gchar *host, gint port, const gchar *secret);
void         aria2_client_free      (Aria2Client *client);

/* Connectivity */
gboolean     aria2_client_is_connected (Aria2Client *client);
gboolean     aria2_client_test_connection (Aria2Client *client);

/* Download control */
void aria2_client_add_uri     (Aria2Client *client, const gchar *url,
                                const gchar *dir, const gchar *out_name, const gchar *referer,
                                gint connections,
                                Aria2Callback cb, gpointer user_data);
void aria2_client_pause       (Aria2Client *client, const gchar *gid,
                                Aria2Callback cb, gpointer user_data);
void aria2_client_resume      (Aria2Client *client, const gchar *gid,
                                Aria2Callback cb, gpointer user_data);
void aria2_client_remove      (Aria2Client *client, const gchar *gid,
                                Aria2Callback cb, gpointer user_data);
void aria2_client_force_remove(Aria2Client *client, const gchar *gid,
                                Aria2Callback cb, gpointer user_data);
void aria2_client_remove_download_result(Aria2Client *client, const gchar *gid,
                                         Aria2Callback cb, gpointer user_data);
void aria2_client_pause_all   (Aria2Client *client, Aria2Callback cb, gpointer user_data);
void aria2_client_resume_all  (Aria2Client *client, Aria2Callback cb, gpointer user_data);

/* Status queries (synchronous, for polling) */
DownloadItem *aria2_client_tell_status     (Aria2Client *client, const gchar *gid);
GList        *aria2_client_tell_active     (Aria2Client *client);
GList        *aria2_client_tell_waiting    (Aria2Client *client, gint offset, gint num);
GList        *aria2_client_tell_stopped    (Aria2Client *client, gint offset, gint num);
gboolean      aria2_client_get_global_stat (Aria2Client *client,
                                             gint64 *dl_speed, gint64 *ul_speed,
                                             gint *num_active, gint *num_waiting, gint *num_stopped);
