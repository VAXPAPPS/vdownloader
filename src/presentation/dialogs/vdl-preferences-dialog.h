/* VXAPDownloader - Preferences Dialog (stub) */
#ifndef VDL_PREFERENCES_DIALOG_H
#define VDL_PREFERENCES_DIALOG_H
#include <adwaita.h>
G_BEGIN_DECLS
#define VDL_TYPE_PREFERENCES_DIALOG (vdl_preferences_dialog_get_type())
G_DECLARE_FINAL_TYPE(VdlPreferencesDialog,vdl_preferences_dialog,VDL,PREFERENCES_DIALOG,AdwWindow)
VdlPreferencesDialog *vdl_preferences_dialog_new(GtkWindow *parent);
G_END_DECLS
#endif
