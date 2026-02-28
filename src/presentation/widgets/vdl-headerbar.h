/*
 * VXAPDownloader - Custom Header Bar (vaxpOS-style title bar)
 */

#ifndef VDL_HEADERBAR_H
#define VDL_HEADERBAR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VDL_TYPE_HEADERBAR (vdl_headerbar_get_type())
G_DECLARE_FINAL_TYPE (VdlHeaderBar, vdl_headerbar, VDL, HEADERBAR, GtkBox)

VdlHeaderBar *vdl_headerbar_new          (GtkWindow *window);
void          vdl_headerbar_set_title    (VdlHeaderBar *self, const char *title);
void          vdl_headerbar_set_subtitle (VdlHeaderBar *self, const char *subtitle);
GtkWidget    *vdl_headerbar_get_start_box(VdlHeaderBar *self);
GtkWidget    *vdl_headerbar_get_end_box  (VdlHeaderBar *self);

G_END_DECLS

#endif /* VDL_HEADERBAR_H */
