#pragma once

#include <glib-object.h>
#include "aria2/aria2_client.h"
#include "aria2/aria2_monitor.h"

#define VXAP_TYPE_DOWNLOAD_MANAGER (download_manager_get_type())
G_DECLARE_FINAL_TYPE(DownloadManager, download_manager, VXAP, DOWNLOAD_MANAGER, GObject)

/* Signals:
 *  "item-added"    : DownloadItem *item
 *  "item-updated"  : DownloadItem *item
 *  "item-removed"  : const gchar *gid
 *  "stats-changed" : dl_speed (gint64), ul_speed (gint64), num_active (gint)
 */

DownloadManager *download_manager_new         (Aria2Client *client);
void             download_manager_start       (DownloadManager *self);
void             download_manager_stop        (DownloadManager *self);

void             download_manager_add_url     (DownloadManager *self,
                                               const gchar *url,
                                               const gchar *dir,
                                               const gchar *out_name,
                                               const gchar *referer,
                                               gint connections);
void             download_manager_pause       (DownloadManager *self, const gchar *gid);
void             download_manager_resume      (DownloadManager *self, const gchar *gid);
void             download_manager_remove      (DownloadManager *self, const gchar *gid);
void             download_manager_pause_all   (DownloadManager *self);
void             download_manager_resume_all  (DownloadManager *self);

/* Returns a GListModel (GtkStringList or GPtrArray-backed) --- current snapshot */
GListModel      *download_manager_get_model   (DownloadManager *self);
