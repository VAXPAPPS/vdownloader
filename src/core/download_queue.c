#include "core/download_queue.h"
#include <glib.h>

typedef struct {
    DownloadItem *item;
    gint priority;
} QueueEntry;

struct _DownloadQueue {
    GQueue *q;
};

DownloadQueue *
download_queue_new (void)
{
    DownloadQueue *queue = g_new0 (DownloadQueue, 1);
    queue->q = g_queue_new ();
    return queue;
}

void
download_queue_free (DownloadQueue *queue)
{
    if (!queue) return;
    g_queue_free_full (queue->q, (GDestroyNotify)g_free);
    g_free (queue);
}

static gint _cmp_priority (gconstpointer a, gconstpointer b, gpointer _)
{
    return ((QueueEntry*)b)->priority - ((QueueEntry*)a)->priority;
}

void
download_queue_push (DownloadQueue *queue, DownloadItem *item, gint priority)
{
    QueueEntry *e = g_new (QueueEntry, 1);
    e->item = item;
    e->priority = priority;
    g_queue_insert_sorted (queue->q, e, _cmp_priority, NULL);
}

DownloadItem *
download_queue_pop (DownloadQueue *queue)
{
    QueueEntry *e = g_queue_pop_head (queue->q);
    if (!e) return NULL;
    DownloadItem *item = e->item;
    g_free (e);
    return item;
}

DownloadItem *
download_queue_peek (DownloadQueue *queue)
{
    QueueEntry *e = g_queue_peek_head (queue->q);
    return e ? e->item : NULL;
}

guint
download_queue_length (DownloadQueue *queue)
{
    return g_queue_get_length (queue->q);
}

void
download_queue_remove (DownloadQueue *queue, const gchar *gid)
{
    for (GList *l = queue->q->head; l; l = l->next) {
        QueueEntry *e = l->data;
        if (g_strcmp0 (e->item->gid, gid) == 0) {
            g_queue_delete_link (queue->q, l);
            download_item_free (e->item);
            g_free (e);
            return;
        }
    }
}
