#pragma once

#include <gtk/gtk.h>
#include "aria2/aria2_types.h"

/* A GtkBox subclass representing one row in the download list */
#define VXAP_TYPE_DOWNLOAD_ROW (download_row_get_type())
G_DECLARE_FINAL_TYPE(DownloadRow, download_row, VXAP, DOWNLOAD_ROW, GtkBox)

GtkWidget *download_row_new    (DownloadItem *item);
void       download_row_update (DownloadRow *self, DownloadItem *item);
const gchar *download_row_get_gid (DownloadRow *self);
gboolean   download_row_is_paused (DownloadRow *self);
