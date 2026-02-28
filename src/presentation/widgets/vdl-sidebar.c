/*
 * VXAPDownloader - Sidebar Navigation
 * Categories sidebar matching VAXP-OS ecosystem design
 */

#include "vdl-sidebar.h"

typedef struct {
    VdlViewType  view;
    const char  *icon_name;
    const char  *label;
    GtkWidget   *button;
    GtkWidget   *badge;
} SidebarEntry;

struct _VdlSidebar {
    GtkBox        parent_instance;

    SidebarEntry  entries[8];
    int           entry_count;
    VdlViewType   active_view;
};

G_DEFINE_TYPE (VdlSidebar, vdl_sidebar, GTK_TYPE_BOX)

static void
on_item_clicked (GtkButton *button, gpointer user_data)
{
    VdlSidebar *self = VDL_SIDEBAR (g_object_get_data (G_OBJECT (button), "sidebar"));
    VdlViewType view = GPOINTER_TO_INT (user_data);
    vdl_sidebar_set_active (self, view);

    /* Emit signal to switch view */
    g_signal_emit_by_name (self, "view-changed", view);
}

static GtkWidget *
create_sidebar_item (VdlSidebar *self, const char *icon_name, const char *label,
                     VdlViewType view, SidebarEntry *entry)
{
    GtkWidget *btn = gtk_button_new ();
    gtk_widget_add_css_class (btn, "vdl-sidebar-item");
    gtk_button_set_has_frame (GTK_BUTTON (btn), FALSE);

    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    /* Icon */
    GtkWidget *icon = gtk_image_new_from_icon_name (icon_name);
    gtk_image_set_pixel_size (GTK_IMAGE (icon), 18);
    gtk_widget_set_margin_end (icon, 12);
    gtk_box_append (GTK_BOX (hbox), icon);

    /* Label */
    GtkWidget *lbl = gtk_label_new (label);
    gtk_widget_set_hexpand (lbl, TRUE);
    gtk_widget_set_halign (lbl, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (hbox), lbl);

    /* Badge */
    GtkWidget *badge = gtk_label_new ("");
    gtk_widget_add_css_class (badge, "vdl-sidebar-badge");
    gtk_widget_set_visible (badge, FALSE);
    gtk_box_append (GTK_BOX (hbox), badge);

    gtk_button_set_child (GTK_BUTTON (btn), hbox);

    g_object_set_data (G_OBJECT (btn), "sidebar", self);
    g_signal_connect (btn, "clicked", G_CALLBACK (on_item_clicked), GINT_TO_POINTER (view));

    entry->view = view;
    entry->icon_name = icon_name;
    entry->label = label;
    entry->button = btn;
    entry->badge = badge;

    return btn;
}

static GtkWidget *
create_section_header (const char *text)
{
    GtkWidget *label = gtk_label_new (text);
    gtk_widget_add_css_class (label, "vdl-sidebar-header");
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    return label;
}

static GtkWidget *
create_separator (void)
{
    GtkWidget *sep = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_add_css_class (sep, "vdl-sidebar-separator");
    return sep;
}

static void
vdl_sidebar_class_init (VdlSidebarClass *klass)
{
    g_signal_new ("view-changed",
                  VDL_TYPE_SIDEBAR,
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  NULL,
                  G_TYPE_NONE, 1,
                  G_TYPE_INT);
}

static void
vdl_sidebar_init (VdlSidebar *self)
{
    gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class (GTK_WIDGET (self), "vdl-sidebar");

    self->entry_count = 0;
    self->active_view = VDL_VIEW_DASHBOARD;

    /* OVERVIEW section */
    gtk_box_append (GTK_BOX (self), create_section_header ("OVERVIEW"));

    GtkWidget *item;
    item = create_sidebar_item (self, "go-home-symbolic", "Dashboard",
                                VDL_VIEW_DASHBOARD, &self->entries[0]);
    gtk_box_append (GTK_BOX (self), item);

    item = create_sidebar_item (self, "folder-download-symbolic", "All Downloads",
                                VDL_VIEW_ALL_DOWNLOADS, &self->entries[1]);
    gtk_box_append (GTK_BOX (self), item);

    item = create_sidebar_item (self, "emblem-synchronizing-symbolic", "Active",
                                VDL_VIEW_ACTIVE, &self->entries[2]);
    gtk_box_append (GTK_BOX (self), item);

    item = create_sidebar_item (self, "emblem-ok-symbolic", "Completed",
                                VDL_VIEW_COMPLETED, &self->entries[3]);
    gtk_box_append (GTK_BOX (self), item);

    self->entry_count = 4;

    /* Separator */
    gtk_box_append (GTK_BOX (self), create_separator ());

    /* TORRENTS section */
    gtk_box_append (GTK_BOX (self), create_section_header ("TORRENTS"));

    item = create_sidebar_item (self, "network-transmit-receive-symbolic", "Torrents",
                                VDL_VIEW_TORRENTS, &self->entries[4]);
    gtk_box_append (GTK_BOX (self), item);
    self->entry_count = 5;

    /* Separator */
    gtk_box_append (GTK_BOX (self), create_separator ());

    /* TOOLS section */
    gtk_box_append (GTK_BOX (self), create_section_header ("TOOLS"));

    item = create_sidebar_item (self, "x-office-spreadsheet-symbolic", "Statistics",
                                VDL_VIEW_STATISTICS, &self->entries[5]);
    gtk_box_append (GTK_BOX (self), item);
    self->entry_count = 6;

    /* Spacer to push settings to bottom */
    GtkWidget *spacer = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand (spacer, TRUE);
    gtk_box_append (GTK_BOX (self), spacer);

    /* Settings at bottom */
    gtk_box_append (GTK_BOX (self), create_separator ());
    item = create_sidebar_item (self, "emblem-system-symbolic", "Settings",
                                VDL_VIEW_SETTINGS, &self->entries[6]);
    gtk_box_append (GTK_BOX (self), item);
    self->entry_count = 7;

    /* Set dashboard as default active */
    vdl_sidebar_set_active (self, VDL_VIEW_DASHBOARD);
}

VdlSidebar *
vdl_sidebar_new (void)
{
    return g_object_new (VDL_TYPE_SIDEBAR, NULL);
}

void
vdl_sidebar_set_active (VdlSidebar *self, VdlViewType view)
{
    g_return_if_fail (VDL_IS_SIDEBAR (self));

    for (int i = 0; i < self->entry_count; i++) {
        if (self->entries[i].view == self->active_view) {
            gtk_widget_remove_css_class (self->entries[i].button, "active");
        }
        if (self->entries[i].view == view) {
            gtk_widget_add_css_class (self->entries[i].button, "active");
        }
    }

    self->active_view = view;
}

void
vdl_sidebar_update_badge (VdlSidebar *self, VdlViewType view, int count)
{
    g_return_if_fail (VDL_IS_SIDEBAR (self));

    for (int i = 0; i < self->entry_count; i++) {
        if (self->entries[i].view == view) {
            if (count > 0) {
                g_autofree char *text = g_strdup_printf ("%d", count);
                gtk_label_set_text (GTK_LABEL (self->entries[i].badge), text);
                gtk_widget_set_visible (self->entries[i].badge, TRUE);
            } else {
                gtk_widget_set_visible (self->entries[i].badge, FALSE);
            }
            break;
        }
    }
}
