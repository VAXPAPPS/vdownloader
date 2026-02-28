/*
 * VXAPDownloader - Main Window
 */

#ifndef VDL_MAIN_WINDOW_H
#define VDL_MAIN_WINDOW_H

#include <adwaita.h>
#include "../../core/vdl-types.h"

G_BEGIN_DECLS

#define VDL_TYPE_MAIN_WINDOW (vdl_main_window_get_type())
G_DECLARE_FINAL_TYPE (VdlMainWindow, vdl_main_window, VDL, MAIN_WINDOW, AdwApplicationWindow)

VdlMainWindow *vdl_main_window_new (VdlApplication *app);
void           vdl_main_window_show_add_dialog (VdlMainWindow *self, const char *url);

G_END_DECLS

#endif /* VDL_MAIN_WINDOW_H */
