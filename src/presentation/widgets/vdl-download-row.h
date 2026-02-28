/* VXAPDownloader - Download Row Widget */
#ifndef VDL_DOWNLOAD_ROW_H
#define VDL_DOWNLOAD_ROW_H
#include <gtk/gtk.h>
#include "../../core/vdl-enums.h"

G_BEGIN_DECLS

#define VDL_TYPE_DOWNLOAD_ROW (vdl_download_row_get_type())
G_DECLARE_FINAL_TYPE (VdlDownloadRow, vdl_download_row, VDL, DOWNLOAD_ROW, GtkBox)

GtkWidget *vdl_download_row_new             (const char *filename, const char *url,
                                              int64_t total_size, const char *save_path,
                                              const char *download_id);
void       vdl_download_row_update_progress (VdlDownloadRow *self,
                                              int64_t downloaded, int64_t total,
                                              double speed);
void       vdl_download_row_set_status      (VdlDownloadRow *self, VdlDownloadStatus status);
const char*vdl_download_row_get_download_id (VdlDownloadRow *self);

/*
 * Signals:
 *   "pause-clicked"   (VdlDownloadRow *row, const char *download_id)
 *   "cancel-clicked"  (VdlDownloadRow *row, const char *download_id)
 */

G_END_DECLS

#endif
