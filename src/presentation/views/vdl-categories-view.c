/*
 * VXAPDownloader - Categories View (stub)
 */

#include "vdl-categories-view.h"

struct _VdlCategoriesView {
    GtkBox parent_instance;
};

G_DEFINE_TYPE (VdlCategoriesView, vdl_categories_view, GTK_TYPE_BOX)

static void vdl_categories_view_class_init (VdlCategoriesViewClass *klass) {}
static void vdl_categories_view_init (VdlCategoriesView *self)
{
    gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class (GTK_WIDGET (self), "vdl-content");

    GtkWidget *lbl = gtk_label_new ("Categories");
    gtk_widget_add_css_class (lbl, "vdl-content-title");
    gtk_widget_set_margin_start (lbl, 24);
    gtk_widget_set_margin_top (lbl, 20);
    gtk_widget_set_halign (lbl, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (self), lbl);
}

GtkWidget *
vdl_categories_view_new (void)
{
    return GTK_WIDGET (g_object_new (VDL_TYPE_CATEGORIES_VIEW, NULL));
}
