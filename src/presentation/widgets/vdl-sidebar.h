/*
 * VXAPDownloader - Sidebar Navigation
 */

#ifndef VDL_SIDEBAR_H
#define VDL_SIDEBAR_H

#include <gtk/gtk.h>
#include "../../core/vdl-enums.h"

G_BEGIN_DECLS

#define VDL_TYPE_SIDEBAR (vdl_sidebar_get_type())
G_DECLARE_FINAL_TYPE (VdlSidebar, vdl_sidebar, VDL, SIDEBAR, GtkBox)

VdlSidebar *vdl_sidebar_new          (void);
void        vdl_sidebar_set_active   (VdlSidebar *self, VdlViewType view);
void        vdl_sidebar_update_badge (VdlSidebar *self, VdlViewType view, int count);

G_END_DECLS

#endif /* VDL_SIDEBAR_H */
