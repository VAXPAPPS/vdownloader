/*
 * VXAPDownloader - Statistics View
 */

#ifndef VDL_STATISTICS_VIEW_H
#define VDL_STATISTICS_VIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VDL_TYPE_STATISTICS_VIEW (vdl_statistics_view_get_type())
G_DECLARE_FINAL_TYPE (VdlStatisticsView, vdl_statistics_view, VDL, STATISTICS_VIEW, GtkBox)

GtkWidget *vdl_statistics_view_new (void);

G_END_DECLS

#endif
