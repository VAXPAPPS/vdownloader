/*
 * VXAPDownloader - Statistics View
 */

#include "vdl-statistics-view.h"

struct _VdlStatisticsView {
    GtkBox parent_instance;
};

G_DEFINE_TYPE (VdlStatisticsView, vdl_statistics_view, GTK_TYPE_BOX)

static void vdl_statistics_view_class_init (VdlStatisticsViewClass *klass) {}
static void vdl_statistics_view_init (VdlStatisticsView *self)
{
    gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class (GTK_WIDGET (self), "vdl-content");

    GtkWidget *header = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class (header, "vdl-content-header");

    GtkWidget *title = gtk_label_new ("Statistics");
    gtk_widget_add_css_class (title, "vdl-content-title");
    gtk_widget_set_halign (title, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (header), title);

    GtkWidget *subtitle = gtk_label_new ("Download history and analytics");
    gtk_widget_add_css_class (subtitle, "vdl-content-subtitle");
    gtk_widget_set_halign (subtitle, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (header), subtitle);

    gtk_box_append (GTK_BOX (self), header);

    /* Stat cards */
    GtkWidget *cards = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_margin_start (cards, 24);
    gtk_widget_set_margin_end (cards, 24);
    gtk_widget_set_margin_top (cards, 20);

    const char *labels[] = {"Total Downloaded", "Total Uploaded", "Files Downloaded", "Average Speed"};
    const char *values[] = {"0 B", "0 B", "0", "0 B/s"};

    for (int i = 0; i < 4; i++) {
        GtkWidget *card = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
        gtk_widget_add_css_class (card, "vdl-stat-card");
        gtk_widget_set_hexpand (card, TRUE);

        GtkWidget *val = gtk_label_new (values[i]);
        gtk_widget_add_css_class (val, "vdl-stat-value");
        gtk_widget_set_halign (val, GTK_ALIGN_START);
        gtk_box_append (GTK_BOX (card), val);

        GtkWidget *lbl = gtk_label_new (labels[i]);
        gtk_widget_add_css_class (lbl, "vdl-stat-label");
        gtk_widget_set_halign (lbl, GTK_ALIGN_START);
        gtk_box_append (GTK_BOX (card), lbl);

        gtk_box_append (GTK_BOX (cards), card);
    }

    gtk_box_append (GTK_BOX (self), cards);
}

GtkWidget *
vdl_statistics_view_new (void)
{
    return GTK_WIDGET (g_object_new (VDL_TYPE_STATISTICS_VIEW, NULL));
}
