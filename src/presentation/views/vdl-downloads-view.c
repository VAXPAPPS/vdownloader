/*
 * VXAPDownloader - Downloads View
 * Shows all downloads with live progress, forward pause/cancel signals.
 */

#include "vdl-downloads-view.h"
#include "../widgets/vdl-download-row.h"
#include "../../core/vdl-utils.h"

struct _VdlDownloadsView {
    GtkBox       parent_instance;
    GtkWidget   *search_entry;
    GtkWidget   *list_box;
    GtkWidget   *empty_state;
    GtkWidget   *count_label;
    int          download_count;
};

G_DEFINE_TYPE (VdlDownloadsView, vdl_downloads_view, GTK_TYPE_BOX)

enum {
    SIGNAL_PAUSE_DOWNLOAD,
    SIGNAL_CANCEL_DOWNLOAD,
    N_VIEW_SIGNALS
};
static guint view_signals[N_VIEW_SIGNALS] = { 0 };

static void update_empty_state (VdlDownloadsView *self) {
    gtk_widget_set_visible (self->empty_state, self->download_count == 0);
}

static void update_count_label (VdlDownloadsView *self) {
    g_autofree char *text = g_strdup_printf ("%d downloads", self->download_count);
    gtk_label_set_text (GTK_LABEL (self->count_label), text);
}

static void
on_progress_changed (VdlDownloadItem *item, gint64 downloaded, gint64 total,
                     gdouble speed, gpointer user_data)
{
    VdlDownloadRow *row = VDL_DOWNLOAD_ROW (user_data);
    vdl_download_row_update_progress (row, downloaded, total, speed);
    vdl_download_row_set_status (row, vdl_download_item_get_status (item));
}

/* Forward row signals to view signals */
static void
on_row_pause (VdlDownloadRow *row, const char *id, gpointer user_data)
{
    (void)row;
    VdlDownloadsView *self = VDL_DOWNLOADS_VIEW (user_data);
    g_signal_emit (self, view_signals[SIGNAL_PAUSE_DOWNLOAD], 0, id);
}

static void
on_row_cancel (VdlDownloadRow *row, const char *id, gpointer user_data)
{
    (void)row;
    VdlDownloadsView *self = VDL_DOWNLOADS_VIEW (user_data);
    g_signal_emit (self, view_signals[SIGNAL_CANCEL_DOWNLOAD], 0, id);
}

static void
vdl_downloads_view_class_init (VdlDownloadsViewClass *klass)
{
    (void)klass;
    view_signals[SIGNAL_PAUSE_DOWNLOAD] = g_signal_new (
        "pause-download", VDL_TYPE_DOWNLOADS_VIEW, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
    view_signals[SIGNAL_CANCEL_DOWNLOAD] = g_signal_new (
        "cancel-download", VDL_TYPE_DOWNLOADS_VIEW, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
vdl_downloads_view_init (VdlDownloadsView *self)
{
    gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class (GTK_WIDGET (self), "vdl-content");
    self->download_count = 0;

    GtkWidget *header = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class (header, "vdl-content-header");

    GtkWidget *title_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand (title_box, TRUE);
    GtkWidget *title = gtk_label_new ("All Downloads");
    gtk_widget_add_css_class (title, "vdl-content-title");
    gtk_widget_set_halign (title, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (title_box), title);
    self->count_label = gtk_label_new ("0 downloads");
    gtk_widget_add_css_class (self->count_label, "vdl-content-subtitle");
    gtk_widget_set_halign (self->count_label, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (title_box), self->count_label);
    gtk_box_append (GTK_BOX (header), title_box);

    self->search_entry = gtk_search_entry_new ();
    gtk_widget_set_valign (self->search_entry, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request (self->search_entry, 250, -1);
    gtk_box_append (GTK_BOX (header), self->search_entry);
    gtk_box_append (GTK_BOX (self), header);

    GtkWidget *scrolled = gtk_scrolled_window_new ();
    gtk_widget_set_vexpand (scrolled, TRUE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    self->list_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_top (self->list_box, 8);
    gtk_widget_set_margin_bottom (self->list_box, 8);
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), self->list_box);
    gtk_box_append (GTK_BOX (self), scrolled);

    self->empty_state = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_valign (self->empty_state, GTK_ALIGN_CENTER);
    gtk_widget_set_halign (self->empty_state, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (self->empty_state, 80);
    GtkWidget *eicon = gtk_image_new_from_icon_name ("folder-download-symbolic");
    gtk_image_set_pixel_size (GTK_IMAGE (eicon), 64);
    gtk_widget_set_opacity (eicon, 0.3);
    gtk_box_append (GTK_BOX (self->empty_state), eicon);
    GtkWidget *msg = gtk_label_new ("No downloads yet\nClick '+ New Download' to start");
    gtk_widget_set_opacity (msg, 0.4);
    gtk_label_set_justify (GTK_LABEL (msg), GTK_JUSTIFY_CENTER);
    gtk_box_append (GTK_BOX (self->empty_state), msg);
    gtk_box_append (GTK_BOX (self->list_box), self->empty_state);
}

GtkWidget *vdl_downloads_view_new (void)
{
    return GTK_WIDGET (g_object_new (VDL_TYPE_DOWNLOADS_VIEW, NULL));
}

void
vdl_downloads_view_add_item (VdlDownloadsView *self, VdlDownloadItem *item)
{
    g_return_if_fail (VDL_IS_DOWNLOADS_VIEW (self));
    g_return_if_fail (VDL_IS_DOWNLOAD_ITEM (item));

    const char *filename  = vdl_download_item_get_filename (item);
    const char *url       = vdl_download_item_get_url (item);
    int64_t total         = vdl_download_item_get_total_size (item);
    const char *save_path = vdl_download_item_get_save_path (item);
    const char *id        = vdl_download_item_get_id (item);

    GtkWidget *row = vdl_download_row_new (filename, url, total, save_path, id);

    g_signal_connect (item, "progress-changed",
                      G_CALLBACK (on_progress_changed), row);
    g_signal_connect (row, "pause-clicked",
                      G_CALLBACK (on_row_pause), self);
    g_signal_connect (row, "cancel-clicked",
                      G_CALLBACK (on_row_cancel), self);

    gtk_box_prepend (GTK_BOX (self->list_box), row);
    self->download_count++;
    update_empty_state (self);
    update_count_label (self);
}
