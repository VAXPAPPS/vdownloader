/* VXAPDownloader - Add Download Dialog */
#ifndef VDL_ADD_DOWNLOAD_DIALOG_H
#define VDL_ADD_DOWNLOAD_DIALOG_H
#include <adwaita.h>
G_BEGIN_DECLS
#define VDL_TYPE_ADD_DOWNLOAD_DIALOG (vdl_add_download_dialog_get_type())
G_DECLARE_FINAL_TYPE(VdlAddDownloadDialog,vdl_add_download_dialog,VDL,ADD_DOWNLOAD_DIALOG,AdwWindow)
VdlAddDownloadDialog *vdl_add_download_dialog_new(GtkWindow *parent);
G_END_DECLS
#endif
