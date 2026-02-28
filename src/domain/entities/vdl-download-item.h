/* VXAPDownloader - Download Item Entity */
#ifndef VDL_DOWNLOAD_ITEM_H
#define VDL_DOWNLOAD_ITEM_H
#include <glib-object.h>
#include "../../core/vdl-enums.h"
#include "../../core/vdl-types.h"

G_BEGIN_DECLS

#define VDL_TYPE_DOWNLOAD_ITEM (vdl_download_item_get_type())
G_DECLARE_FINAL_TYPE (VdlDownloadItem, vdl_download_item, VDL, DOWNLOAD_ITEM, GObject)

VdlDownloadItem    *vdl_download_item_new           (const char *url, const char *save_path);
const char         *vdl_download_item_get_id         (VdlDownloadItem *self);
const char         *vdl_download_item_get_url        (VdlDownloadItem *self);
const char         *vdl_download_item_get_filename   (VdlDownloadItem *self);
void                vdl_download_item_set_filename   (VdlDownloadItem *self, const char *name);
const char         *vdl_download_item_get_save_path  (VdlDownloadItem *self);
int64_t             vdl_download_item_get_total_size (VdlDownloadItem *self);
void                vdl_download_item_set_total_size (VdlDownloadItem *self, int64_t size);
int64_t             vdl_download_item_get_downloaded (VdlDownloadItem *self);
void                vdl_download_item_set_downloaded (VdlDownloadItem *self, int64_t bytes);
VdlDownloadStatus   vdl_download_item_get_status     (VdlDownloadItem *self);
void                vdl_download_item_set_status     (VdlDownloadItem *self, VdlDownloadStatus s);
VdlDownloadType     vdl_download_item_get_type_      (VdlDownloadItem *self);
int                 vdl_download_item_get_segments   (VdlDownloadItem *self);
void                vdl_download_item_set_segments   (VdlDownloadItem *self, int segments);
double              vdl_download_item_get_progress   (VdlDownloadItem *self);

/*
 * Signal: "progress-changed"
 *   void callback (VdlDownloadItem *item, gint64 downloaded, gint64 total, gdouble speed, gpointer user_data)
 */

G_END_DECLS

#endif
