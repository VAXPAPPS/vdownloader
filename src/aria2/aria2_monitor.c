#include "aria2_monitor.h"
#include <glib-object.h>

/* ─────────────────────────── GObject ────────────────────────── */

struct _Aria2Monitor {
    GObject parent;
};

typedef struct {
    Aria2Client *client;
    guint        timeout_id;
    gboolean     running;
    guint        interval_ms;
} Aria2MonitorPrivate;

enum { SIGNAL_DOWNLOADS_UPDATED, SIGNAL_STATS_UPDATED, SIGNAL_DOWNLOAD_COMPLETE, N_SIGNALS };
static guint signals[N_SIGNALS];

G_DEFINE_TYPE_WITH_PRIVATE (Aria2Monitor, aria2_monitor, G_TYPE_OBJECT)

static void aria2_monitor_finalize (GObject *obj)
{
    Aria2Monitor *self = VXAP_ARIA2_MONITOR (obj);
    aria2_monitor_stop (self);
    G_OBJECT_CLASS (aria2_monitor_parent_class)->finalize (obj);
}

static void aria2_monitor_class_init (Aria2MonitorClass *klass)
{
    GObjectClass *oc = G_OBJECT_CLASS (klass);
    oc->finalize = aria2_monitor_finalize;

    signals[SIGNAL_DOWNLOADS_UPDATED] = g_signal_new ("downloads-updated",
        G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
        NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);
    signals[SIGNAL_STATS_UPDATED] = g_signal_new ("stats-updated",
        G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
        NULL, NULL, NULL, G_TYPE_NONE, 5,
        G_TYPE_INT64, G_TYPE_INT64, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
    signals[SIGNAL_DOWNLOAD_COMPLETE] = g_signal_new ("download-complete",
        G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
        NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void aria2_monitor_init (Aria2Monitor *self)
{
    Aria2MonitorPrivate *priv = aria2_monitor_get_instance_private (self);
    priv->interval_ms = 1000;
}

/* ─────────────────────────────── Poll ──────────────────────────────── */

static GHashTable *completed_gids = NULL;

static gboolean
_poll_cb (gpointer user_data)
{
    Aria2Monitor *self = VXAP_ARIA2_MONITOR (user_data);
    Aria2MonitorPrivate *priv = aria2_monitor_get_instance_private (self);
    if (!priv->running) return G_SOURCE_REMOVE;

    GList *active   = aria2_client_tell_active   (priv->client);
    GList *waiting  = aria2_client_tell_waiting  (priv->client, 0, 100);
    GList *stopped  = aria2_client_tell_stopped  (priv->client, 0, 200);

    GList *all = NULL;
    all = g_list_concat (all, g_list_copy (active));
    all = g_list_concat (all, g_list_copy (waiting));
    all = g_list_concat (all, g_list_copy (stopped));

    /* Fire download-complete for newly finished items */
    for (GList *l = stopped; l; l = l->next) {
        DownloadItem *item = l->data;
        if (item->state == DOWNLOAD_STATE_COMPLETE &&
            !g_hash_table_contains (completed_gids, item->gid)) {
            g_hash_table_add (completed_gids, g_strdup (item->gid));
            g_signal_emit (self, signals[SIGNAL_DOWNLOAD_COMPLETE], 0, item);
        }
    }

    g_signal_emit (self, signals[SIGNAL_DOWNLOADS_UPDATED], 0, all);

    gint64 dl = 0, ul = 0;
    gint na = 0, nw = 0, ns = 0;
    aria2_client_get_global_stat (priv->client, &dl, &ul, &na, &nw, &ns);
    g_signal_emit (self, signals[SIGNAL_STATS_UPDATED], 0, dl, ul, na, nw, ns);

    g_list_free_full (active,  (GDestroyNotify)download_item_free);
    g_list_free_full (waiting, (GDestroyNotify)download_item_free);
    g_list_free_full (stopped, (GDestroyNotify)download_item_free);
    g_list_free (all); /* shallow free — items already freed above */

    return G_SOURCE_CONTINUE;
}

/* ─────────────────────────────── API ──────────────────────────────── */

Aria2Monitor *
aria2_monitor_new (Aria2Client *client)
{
    if (!completed_gids)
        completed_gids = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    Aria2Monitor *self = g_object_new (VXAP_TYPE_ARIA2_MONITOR, NULL);
    Aria2MonitorPrivate *priv = aria2_monitor_get_instance_private (self);
    priv->client = client;
    return self;
}

void
aria2_monitor_start (Aria2Monitor *self)
{
    Aria2MonitorPrivate *priv = aria2_monitor_get_instance_private (self);
    if (priv->running) return;
    priv->running = TRUE;
    priv->timeout_id = g_timeout_add (priv->interval_ms, _poll_cb, self);
}

void
aria2_monitor_stop (Aria2Monitor *self)
{
    Aria2MonitorPrivate *priv = aria2_monitor_get_instance_private (self);
    if (!priv->running) return;
    priv->running = FALSE;
    if (priv->timeout_id) {
        g_source_remove (priv->timeout_id);
        priv->timeout_id = 0;
    }
}

gboolean
aria2_monitor_is_running (Aria2Monitor *self)
{
    Aria2MonitorPrivate *priv = aria2_monitor_get_instance_private (self);
    return priv->running;
}

void
aria2_monitor_set_interval (Aria2Monitor *self, guint ms)
{
    Aria2MonitorPrivate *priv = aria2_monitor_get_instance_private (self);
    priv->interval_ms = ms;
    if (priv->running) {
        g_source_remove (priv->timeout_id);
        priv->timeout_id = g_timeout_add (ms, _poll_cb, self);
    }
}
