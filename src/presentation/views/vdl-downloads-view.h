/*
 * VXAPDownloader - Downloads View
 * Shows all downloads with live progress. Forwards pause/cancel actions
 * to the main window via signals.
 */

#ifndef VDL_DOWNLOADS_VIEW_H
#define VDL_DOWNLOADS_VIEW_H

#include <gtk/gtk.h>
#include "../../domain/entities/vdl-download-item.h"

G_BEGIN_DECLS

#define VDL_TYPE_DOWNLOADS_VIEW (vdl_downloads_view_get_type())
G_DECLARE_FINAL_TYPE (VdlDownloadsView, vdl_downloads_view, VDL, DOWNLOADS_VIEW, GtkBox)

GtkWidget *vdl_downloads_view_new      (void);
void       vdl_downloads_view_add_item (VdlDownloadsView *self, VdlDownloadItem *item);

/*
 * Signals:
 *   "pause-download"  (VdlDownloadsView *view, const char *download_id)
 *   "cancel-download" (VdlDownloadsView *view, const char *download_id)
 */

G_END_DECLS

#endif
