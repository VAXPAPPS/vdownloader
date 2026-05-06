#include "core/download_manager.h"
#include "core/utils.h"
#include <glib-object.h>

/* ─── Wrapper GObject for GListStore ─── */
typedef struct _DownloadObject {
    GObject parent;
    DownloadItem *item;
} DownloadObject;

typedef struct { GObjectClass parent; } DownloadObjectClass;

GType download_object_get_type (void);
G_DEFINE_TYPE (DownloadObject, download_object, G_TYPE_OBJECT)

static void download_object_finalize (GObject *o)
{
    download_item_free (((DownloadObject*)o)->item);
    G_OBJECT_CLASS (download_object_parent_class)->finalize (o);
}
static void download_object_class_init (DownloadObjectClass *k)
{ G_OBJECT_CLASS(k)->finalize = download_object_finalize; }
static void download_object_init (DownloadObject *o) { (void)o; }

static DownloadObject *download_object_new (DownloadItem *item)
{
    DownloadObject *o = g_object_new (download_object_get_type (), NULL);
    o->item = item;
    return o;
}

/* ─── DownloadManager private data ─── */
struct _DownloadManager {
    GObject parent;
};

typedef struct {
    Aria2Client   *client;
    Aria2Monitor  *monitor;
    GHashTable    *items;   /* gid -> DownloadItem* */
    GListStore    *store;
} DownloadManagerPrivate;

enum { SIG_ITEM_ADDED, SIG_ITEM_UPDATED, SIG_ITEM_REMOVED, SIG_STATS_CHANGED, N_SIGS };
static guint sigs[N_SIGS];

G_DEFINE_TYPE_WITH_PRIVATE (DownloadManager, download_manager, G_TYPE_OBJECT)

static void download_manager_finalize (GObject *obj)
{
    DownloadManager *self = VXAP_DOWNLOAD_MANAGER (obj);
    download_manager_stop (self);
    DownloadManagerPrivate *p = download_manager_get_instance_private (self);
    if (p->monitor) { g_object_unref (p->monitor); p->monitor = NULL; }
    g_hash_table_destroy (p->items);
    g_object_unref (p->store);
    G_OBJECT_CLASS (download_manager_parent_class)->finalize (obj);
}

static void download_manager_class_init (DownloadManagerClass *klass)
{
    GObjectClass *oc = G_OBJECT_CLASS (klass);
    oc->finalize = download_manager_finalize;
    sigs[SIG_ITEM_ADDED]   = g_signal_new ("item-added",   G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,0,NULL,NULL,NULL,G_TYPE_NONE,1,G_TYPE_POINTER);
    sigs[SIG_ITEM_UPDATED] = g_signal_new ("item-updated", G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,0,NULL,NULL,NULL,G_TYPE_NONE,1,G_TYPE_POINTER);
    sigs[SIG_ITEM_REMOVED] = g_signal_new ("item-removed", G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,0,NULL,NULL,NULL,G_TYPE_NONE,1,G_TYPE_STRING);
    sigs[SIG_STATS_CHANGED]= g_signal_new ("stats-changed",G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,0,NULL,NULL,NULL,G_TYPE_NONE,3,G_TYPE_INT64,G_TYPE_INT64,G_TYPE_INT);
}

static void download_manager_init (DownloadManager *self)
{
    DownloadManagerPrivate *p = download_manager_get_instance_private (self);
    p->items = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                      (GDestroyNotify)download_item_free);
    p->store = g_list_store_new (download_object_get_type ());
}

/* ─── Monitor callbacks ─── */
static void
_on_downloads_updated (Aria2Monitor *mon, GList *all, gpointer user_data)
{
    (void)mon;
    DownloadManager *self = VXAP_DOWNLOAD_MANAGER (user_data);
    DownloadManagerPrivate *p = download_manager_get_instance_private (self);

    GHashTable *seen = g_hash_table_new (g_str_hash, g_str_equal);

    for (GList *l = all; l; l = l->next) {
        DownloadItem *item = l->data;
        g_hash_table_add (seen, item->gid);

        if (g_hash_table_contains (p->items, item->gid)) {
            /* Update */
            DownloadItem *copy = download_item_copy (item);
            g_hash_table_replace (p->items, g_strdup (item->gid), copy);
            g_signal_emit (self, sigs[SIG_ITEM_UPDATED], 0, copy);
            /* Update store */
            guint n = g_list_model_get_n_items (G_LIST_MODEL(p->store));
            for (guint i = 0; i < n; i++) {
                DownloadObject *dobj = g_list_model_get_item (G_LIST_MODEL(p->store), i);
                if (g_strcmp0 (dobj->item->gid, item->gid) == 0) {
                    download_item_free (dobj->item);
                    dobj->item = download_item_copy (item);
                    g_object_unref (dobj);
                    break;
                }
                g_object_unref (dobj);
            }
        } else {
            /* New */
            DownloadItem *copy = download_item_copy (item);
            g_hash_table_insert (p->items, g_strdup (item->gid), copy);
            DownloadObject *dobj = download_object_new (download_item_copy (item));
            g_list_store_append (p->store, dobj);
            g_object_unref (dobj);
            g_signal_emit (self, sigs[SIG_ITEM_ADDED], 0, copy);
        }
    }

    /* Remove missing */
    GList *to_remove = NULL;
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init (&iter, p->items);
    while (g_hash_table_iter_next (&iter, &key, &value)) {
        if (!g_hash_table_contains (seen, key))
            to_remove = g_list_append (to_remove, g_strdup (key));
    }

    for (GList *l = to_remove; l; l = l->next) {
        const gchar *gid = l->data;
        guint n = g_list_model_get_n_items (G_LIST_MODEL(p->store));
        for (guint i = 0; i < n; i++) {
            DownloadObject *dobj = g_list_model_get_item (G_LIST_MODEL(p->store), i);
            if (g_strcmp0 (dobj->item->gid, gid) == 0) {
                g_list_store_remove (p->store, i);
                g_object_unref (dobj);
                break;
            }
            g_object_unref (dobj);
        }
        g_hash_table_remove (p->items, gid);
        g_signal_emit (self, sigs[SIG_ITEM_REMOVED], 0, gid);
        g_free (l->data);
    }
    g_list_free (to_remove);
    g_hash_table_destroy (seen);
}

static void
_on_stats_updated (Aria2Monitor *mon, gint64 dl, gint64 ul,
                   gint na, gint nw, gint ns, gpointer user_data)
{
    (void)mon; (void)nw; (void)ns;
    DownloadManager *self = VXAP_DOWNLOAD_MANAGER (user_data);
    g_signal_emit (self, sigs[SIG_STATS_CHANGED], 0, dl, ul, na);
}

/* ─── API ─── */
DownloadManager *
download_manager_new (Aria2Client *client)
{
    DownloadManager *self = g_object_new (VXAP_TYPE_DOWNLOAD_MANAGER, NULL);
    DownloadManagerPrivate *p = download_manager_get_instance_private (self);
    p->client  = client;
    p->monitor = aria2_monitor_new (client);
    g_signal_connect (p->monitor, "downloads-updated", G_CALLBACK(_on_downloads_updated), self);
    g_signal_connect (p->monitor, "stats-updated",     G_CALLBACK(_on_stats_updated),     self);
    return self;
}

void download_manager_start (DownloadManager *self)
{ DownloadManagerPrivate *p = download_manager_get_instance_private(self);
  if (p->monitor) aria2_monitor_start (p->monitor); }

void download_manager_stop (DownloadManager *self)
{ DownloadManagerPrivate *p = download_manager_get_instance_private(self);
  if (p->monitor) aria2_monitor_stop (p->monitor); }

void download_manager_add_url (DownloadManager *self, const gchar *url,
                               const gchar *dir, const gchar *out_name, const gchar *referer, gint connections)
{ DownloadManagerPrivate *p = download_manager_get_instance_private(self);
  aria2_client_add_uri (p->client, url, dir, out_name, referer, connections, NULL, NULL); }

void download_manager_pause   (DownloadManager *self, const gchar *gid)
{ DownloadManagerPrivate *p = download_manager_get_instance_private(self);
  aria2_client_pause        (p->client, gid, NULL, NULL); }

void download_manager_resume  (DownloadManager *self, const gchar *gid)
{ DownloadManagerPrivate *p = download_manager_get_instance_private(self);
  aria2_client_resume       (p->client, gid, NULL, NULL); }

void download_manager_remove  (DownloadManager *self, const gchar *gid)
{ DownloadManagerPrivate *p = download_manager_get_instance_private(self);
  DownloadItem *item = g_hash_table_lookup (p->items, gid);
  if (item) {
      if (item->state == DOWNLOAD_STATE_ACTIVE || item->state == DOWNLOAD_STATE_WAITING || item->state == DOWNLOAD_STATE_PAUSED)
          aria2_client_force_remove (p->client, gid, NULL, NULL);
      else
          aria2_client_remove_download_result (p->client, gid, NULL, NULL);
  }
}

void download_manager_pause_all  (DownloadManager *self)
{ DownloadManagerPrivate *p = download_manager_get_instance_private(self);
  aria2_client_pause_all  (p->client, NULL, NULL); }

void download_manager_resume_all (DownloadManager *self)
{ DownloadManagerPrivate *p = download_manager_get_instance_private(self);
  aria2_client_resume_all (p->client, NULL, NULL); }

GListModel *
download_manager_get_model (DownloadManager *self)
{ DownloadManagerPrivate *p = download_manager_get_instance_private(self);
  return G_LIST_MODEL (p->store); }
