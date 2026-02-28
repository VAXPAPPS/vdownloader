/* VXAPDownloader - Speed Graph Widget */
#ifndef VDL_SPEED_GRAPH_H
#define VDL_SPEED_GRAPH_H
#include <gtk/gtk.h>
G_BEGIN_DECLS
#define VDL_TYPE_SPEED_GRAPH (vdl_speed_graph_get_type())
G_DECLARE_FINAL_TYPE (VdlSpeedGraph, vdl_speed_graph, VDL, SPEED_GRAPH, GtkDrawingArea)
GtkWidget *vdl_speed_graph_new (void);
void vdl_speed_graph_add_sample (VdlSpeedGraph *self, double download_speed, double upload_speed);
G_END_DECLS
#endif
