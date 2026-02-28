/*
 * VXAPDownloader - Torrents View
 * Displays active/completed torrents with live progress updates.
 */

#ifndef VDL_TORRENTS_VIEW_H
#define VDL_TORRENTS_VIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VDL_TYPE_TORRENTS_VIEW (vdl_torrents_view_get_type())
G_DECLARE_FINAL_TYPE (VdlTorrentsView, vdl_torrents_view, VDL, TORRENTS_VIEW, GtkBox)

GtkWidget *vdl_torrents_view_new             (void);
void       vdl_torrents_view_add_torrent     (VdlTorrentsView *self,
                                               const char *name, int index);
void       vdl_torrents_view_update_torrent  (VdlTorrentsView *self, int index,
                                               double progress, double dl_speed,
                                               double ul_speed, int peers,
                                               int seeds, int eta,
                                               const char *status_text);

/*
 * Signal: "add-torrent-clicked" (VdlTorrentsView *view)
 */

G_END_DECLS

#endif
