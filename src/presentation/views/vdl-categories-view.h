/*
 * VXAPDownloader - Categories View
 */

#ifndef VDL_CATEGORIES_VIEW_H
#define VDL_CATEGORIES_VIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VDL_TYPE_CATEGORIES_VIEW (vdl_categories_view_get_type())
G_DECLARE_FINAL_TYPE (VdlCategoriesView, vdl_categories_view, VDL, CATEGORIES_VIEW, GtkBox)

GtkWidget *vdl_categories_view_new (void);

G_END_DECLS

#endif
