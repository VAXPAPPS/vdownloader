#pragma once

#include <glib.h>
#include "aria2/aria2_types.h"

/* A priority queue for pending downloads */
typedef struct _DownloadQueue DownloadQueue;

DownloadQueue *download_queue_new   (void);
void           download_queue_free  (DownloadQueue *queue);

void           download_queue_push  (DownloadQueue *queue, DownloadItem *item, gint priority);
DownloadItem  *download_queue_pop   (DownloadQueue *queue);
DownloadItem  *download_queue_peek  (DownloadQueue *queue);
guint          download_queue_length(DownloadQueue *queue);
void           download_queue_remove(DownloadQueue *queue, const gchar *gid);
