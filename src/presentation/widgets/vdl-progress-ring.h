/* VXAPDownloader - Progress Ring Widget (stub) */
#ifndef VDL_PROGRESS_RING_H
#define VDL_PROGRESS_RING_H
#include <gtk/gtk.h>
G_BEGIN_DECLS
#define VDL_TYPE_PROGRESS_RING (vdl_progress_ring_get_type())
G_DECLARE_FINAL_TYPE (VdlProgressRing, vdl_progress_ring, VDL, PROGRESS_RING, GtkDrawingArea)
GtkWidget *vdl_progress_ring_new (void);
void vdl_progress_ring_set_progress (VdlProgressRing *self, double progress);
G_END_DECLS
#endif
