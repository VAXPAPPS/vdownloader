#include "ui/widgets/sidebar.h"
#include <gtk/gtk.h>

struct _Sidebar {
    GtkBox       parent;
    gchar       *active_filter;
    GtkListBox  *list;
};

enum { SIG_FILTER_CHANGED, N_SIGS };
static guint sigs[N_SIGS];

G_DEFINE_FINAL_TYPE (Sidebar, sidebar, GTK_TYPE_BOX)

static void sidebar_finalize (GObject *o)
{
    g_free (VXAP_SIDEBAR(o)->active_filter);
    G_OBJECT_CLASS(sidebar_parent_class)->finalize (o);
}

static void sidebar_class_init (SidebarClass *k)
{
    G_OBJECT_CLASS(k)->finalize = sidebar_finalize;
    sigs[SIG_FILTER_CHANGED] = g_signal_new ("filter-changed",
        G_TYPE_FROM_CLASS(k), G_SIGNAL_RUN_LAST, 0,
        NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void sidebar_init (Sidebar *self) { self->active_filter = g_strdup ("all"); }

typedef struct { const gchar *id; const gchar *icon; const gchar *label; } SidebarItem;
static const SidebarItem ITEMS[] = {
    { "all",      "folder-symbolic",               "All"      },
    { "active",   "network-transmit-symbolic",     "Active"   },
    { "paused",   "media-playback-pause-symbolic", "Paused"   },
    { "complete", "emblem-ok-symbolic",            "Complete" },
    { "error",    "dialog-error-symbolic",         "Error"    },
};

static void
_row_activated (GtkListBox *lb, GtkListBoxRow *row, gpointer user_data)
{
    (void)lb;
    Sidebar *self = VXAP_SIDEBAR (user_data);
    const gchar *filter_id = g_object_get_data (G_OBJECT(row), "filter-id");
    g_free (self->active_filter);
    self->active_filter = g_strdup (filter_id);
    g_signal_emit (self, sigs[SIG_FILTER_CHANGED], 0, filter_id);
}

GtkWidget *
sidebar_new (void)
{
    Sidebar *self = g_object_new (VXAP_TYPE_SIDEBAR,
                                  "orientation", GTK_ORIENTATION_VERTICAL,
                                  "spacing", 0, NULL);
    gtk_widget_add_css_class (GTK_WIDGET(self), "sidebar");
    gtk_widget_set_size_request (GTK_WIDGET(self), 200, -1);

    GtkLabel *hdr = GTK_LABEL (gtk_label_new ("VXAP Downloader"));
    gtk_widget_add_css_class (GTK_WIDGET(hdr), "sidebar-header");
    gtk_label_set_xalign (hdr, 0);
    gtk_widget_set_margin_start  (GTK_WIDGET(hdr), 16);
    gtk_widget_set_margin_top    (GTK_WIDGET(hdr), 20);
    gtk_widget_set_margin_bottom (GTK_WIDGET(hdr), 12);
    gtk_box_append (GTK_BOX(self), GTK_WIDGET(hdr));
    gtk_box_append (GTK_BOX(self),
        GTK_WIDGET(gtk_separator_new (GTK_ORIENTATION_HORIZONTAL)));

    self->list = GTK_LIST_BOX (gtk_list_box_new ());
    gtk_list_box_set_selection_mode (self->list, GTK_SELECTION_SINGLE);
    gtk_widget_add_css_class (GTK_WIDGET(self->list), "navigation-sidebar");
    gtk_widget_set_vexpand (GTK_WIDGET(self->list), TRUE);

    for (gsize i = 0; i < G_N_ELEMENTS(ITEMS); i++) {
        GtkBox *row_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10));
        gtk_widget_set_margin_start  (GTK_WIDGET(row_box), 12);
        gtk_widget_set_margin_end    (GTK_WIDGET(row_box), 12);
        gtk_widget_set_margin_top    (GTK_WIDGET(row_box), 8);
        gtk_widget_set_margin_bottom (GTK_WIDGET(row_box), 8);
        gtk_box_append (row_box, GTK_WIDGET(gtk_image_new_from_icon_name (ITEMS[i].icon)));
        GtkLabel *lbl = GTK_LABEL (gtk_label_new (ITEMS[i].label));
        gtk_label_set_xalign (lbl, 0);
        gtk_widget_set_hexpand (GTK_WIDGET(lbl), TRUE);
        gtk_box_append (row_box, GTK_WIDGET(lbl));

        GtkListBoxRow *row = GTK_LIST_BOX_ROW (gtk_list_box_row_new ());
        gtk_list_box_row_set_child (row, GTK_WIDGET(row_box));
        g_object_set_data_full (G_OBJECT(row), "filter-id", g_strdup (ITEMS[i].id), g_free);
        gtk_list_box_append (self->list, GTK_WIDGET(row));
    }
    gtk_list_box_select_row (self->list, gtk_list_box_get_row_at_index (self->list, 0));
    g_signal_connect (self->list, "row-activated", G_CALLBACK(_row_activated), self);
    gtk_box_append (GTK_BOX(self), GTK_WIDGET(self->list));
    return GTK_WIDGET(self);
}

const gchar *
sidebar_get_active_filter (Sidebar *self)
{
    return self->active_filter;
}
