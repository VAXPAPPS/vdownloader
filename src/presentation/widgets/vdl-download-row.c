/*
 * VXAPDownloader - Download Row Widget
 * Individual download item with progress bar, speed, ETA, status badge,
 * action buttons. Emits signals for pause/cancel actions.
 */

#include "vdl-download-row.h"
#include "../../core/vdl-utils.h"

struct _VdlDownloadRow {
    GtkBox             parent_instance;

    char              *filename;
    char              *url;
    char              *save_path;
    char              *download_id;
    VdlDownloadStatus  status;
    gboolean           is_paused;

    GtkWidget         *filename_label;
    GtkWidget         *info_label;
    GtkWidget         *speed_label;
    GtkWidget         *progress_bar;
    GtkWidget         *status_badge;
    GtkWidget         *pause_btn;
    GtkWidget         *cancel_btn;
    GtkWidget         *folder_btn;
    GtkWidget         *actions_box;
};

G_DEFINE_TYPE (VdlDownloadRow, vdl_download_row, GTK_TYPE_BOX)

enum {
    SIGNAL_PAUSE_CLICKED,
    SIGNAL_CANCEL_CLICKED,
    N_SIGNALS
};
static guint row_signals[N_SIGNALS] = { 0 };

static const char *
status_to_string (VdlDownloadStatus status)
{
    switch (status) {
        case VDL_DOWNLOAD_STATUS_QUEUED:       return "Queued";
        case VDL_DOWNLOAD_STATUS_DOWNLOADING:  return "Downloading";
        case VDL_DOWNLOAD_STATUS_PAUSED:       return "Paused";
        case VDL_DOWNLOAD_STATUS_COMPLETED:    return "Completed";
        case VDL_DOWNLOAD_STATUS_ERROR:        return "Error";
        case VDL_DOWNLOAD_STATUS_MERGING:      return "Merging";
        case VDL_DOWNLOAD_STATUS_VERIFYING:    return "Verifying";
        case VDL_DOWNLOAD_STATUS_SEEDING:      return "Seeding";
        default: return "Unknown";
    }
}

static const char *
status_to_css_class (VdlDownloadStatus status)
{
    switch (status) {
        case VDL_DOWNLOAD_STATUS_DOWNLOADING:  return "vdl-badge-downloading";
        case VDL_DOWNLOAD_STATUS_COMPLETED:    return "vdl-badge-completed";
        case VDL_DOWNLOAD_STATUS_PAUSED:       return "vdl-badge-paused";
        case VDL_DOWNLOAD_STATUS_ERROR:        return "vdl-badge-error";
        case VDL_DOWNLOAD_STATUS_SEEDING:      return "vdl-badge-seeding";
        default: return "vdl-badge-downloading";
    }
}

/* ── Button Handlers ── */
static void
on_pause_clicked (GtkButton *btn, gpointer user_data)
{
    (void)btn;
    VdlDownloadRow *self = VDL_DOWNLOAD_ROW (user_data);
    g_signal_emit (self, row_signals[SIGNAL_PAUSE_CLICKED], 0, self->download_id);
}

static void
on_cancel_clicked (GtkButton *btn, gpointer user_data)
{
    (void)btn;
    VdlDownloadRow *self = VDL_DOWNLOAD_ROW (user_data);
    g_signal_emit (self, row_signals[SIGNAL_CANCEL_CLICKED], 0, self->download_id);
}

static void
on_show_folder_clicked (GtkButton *btn, gpointer user_data)
{
    (void)btn;
    VdlDownloadRow *self = VDL_DOWNLOAD_ROW (user_data);
    g_autofree char *uri = g_filename_to_uri (self->save_path, NULL, NULL);
    if (uri) {
        GtkUriLauncher *launcher = gtk_uri_launcher_new (uri);
        gtk_uri_launcher_launch (launcher,
            GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (self))),
            NULL, NULL, NULL);
        g_object_unref (launcher);
    }
}

/* ── GObject ── */
static void
vdl_download_row_finalize (GObject *obj)
{
    VdlDownloadRow *self = VDL_DOWNLOAD_ROW (obj);
    g_free (self->filename);
    g_free (self->url);
    g_free (self->save_path);
    g_free (self->download_id);
    G_OBJECT_CLASS (vdl_download_row_parent_class)->finalize (obj);
}

static void
vdl_download_row_class_init (VdlDownloadRowClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS (klass);
    obj_class->finalize = vdl_download_row_finalize;

    row_signals[SIGNAL_PAUSE_CLICKED] = g_signal_new (
        "pause-clicked", VDL_TYPE_DOWNLOAD_ROW, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);

    row_signals[SIGNAL_CANCEL_CLICKED] = g_signal_new (
        "cancel-clicked", VDL_TYPE_DOWNLOAD_ROW, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
vdl_download_row_init (VdlDownloadRow *self)
{
    gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class (GTK_WIDGET (self), "vdl-download-row");
    self->is_paused = FALSE;
}

GtkWidget *
vdl_download_row_new (const char *filename, const char *url,
                      int64_t total_size, const char *save_path,
                      const char *download_id)
{
    (void)total_size;
    VdlDownloadRow *self = g_object_new (VDL_TYPE_DOWNLOAD_ROW, NULL);
    self->filename    = g_strdup (filename);
    self->url         = g_strdup (url);
    self->save_path   = g_strdup (save_path);
    self->download_id = g_strdup (download_id);
    self->status      = VDL_DOWNLOAD_STATUS_QUEUED;

    /* ── Top row ── */
    GtkWidget *top_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_bottom (top_row, 4);

    GtkWidget *icon = gtk_image_new_from_icon_name ("text-x-generic-symbolic");
    gtk_image_set_pixel_size (GTK_IMAGE (icon), 20);
    gtk_widget_set_opacity (icon, 0.6);
    gtk_box_append (GTK_BOX (top_row), icon);

    self->filename_label = gtk_label_new (filename);
    gtk_widget_add_css_class (self->filename_label, "vdl-download-filename");
    gtk_widget_set_hexpand (self->filename_label, TRUE);
    gtk_widget_set_halign (self->filename_label, GTK_ALIGN_START);
    gtk_label_set_ellipsize (GTK_LABEL (self->filename_label), PANGO_ELLIPSIZE_MIDDLE);
    gtk_box_append (GTK_BOX (top_row), self->filename_label);

    self->status_badge = gtk_label_new ("Queued");
    gtk_widget_add_css_class (self->status_badge, "vdl-badge");
    gtk_widget_add_css_class (self->status_badge, "vdl-badge-downloading");
    gtk_widget_set_valign (self->status_badge, GTK_ALIGN_CENTER);
    gtk_box_append (GTK_BOX (top_row), self->status_badge);

    /* Actions */
    self->actions_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_append (GTK_BOX (top_row), self->actions_box);

    self->pause_btn = gtk_button_new_from_icon_name ("media-playback-pause-symbolic");
    gtk_widget_add_css_class (self->pause_btn, "vdl-action-btn");
    gtk_widget_set_tooltip_text (self->pause_btn, "Pause");
    g_signal_connect (self->pause_btn, "clicked", G_CALLBACK (on_pause_clicked), self);
    gtk_box_append (GTK_BOX (self->actions_box), self->pause_btn);

    self->cancel_btn = gtk_button_new_from_icon_name ("process-stop-symbolic");
    gtk_widget_add_css_class (self->cancel_btn, "vdl-action-btn");
    gtk_widget_add_css_class (self->cancel_btn, "destructive");
    gtk_widget_set_tooltip_text (self->cancel_btn, "Cancel");
    g_signal_connect (self->cancel_btn, "clicked", G_CALLBACK (on_cancel_clicked), self);
    gtk_box_append (GTK_BOX (self->actions_box), self->cancel_btn);

    self->folder_btn = gtk_button_new_from_icon_name ("folder-open-symbolic");
    gtk_widget_add_css_class (self->folder_btn, "vdl-action-btn");
    gtk_widget_add_css_class (self->folder_btn, "vdl-folder-btn");
    gtk_widget_set_tooltip_text (self->folder_btn, "Show in Folder");
    gtk_widget_set_visible (self->folder_btn, FALSE);
    g_signal_connect (self->folder_btn, "clicked", G_CALLBACK (on_show_folder_clicked), self);
    gtk_box_append (GTK_BOX (self->actions_box), self->folder_btn);

    gtk_box_append (GTK_BOX (self), top_row);

    /* ── Progress bar ── */
    self->progress_bar = gtk_progress_bar_new ();
    gtk_widget_add_css_class (self->progress_bar, "vdl-progress");
    gtk_box_append (GTK_BOX (self), self->progress_bar);

    /* ── Bottom row ── */
    GtkWidget *bottom_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_top (bottom_row, 2);

    self->speed_label = gtk_label_new ("Waiting...");
    gtk_widget_add_css_class (self->speed_label, "vdl-download-speed");
    gtk_box_append (GTK_BOX (bottom_row), self->speed_label);

    self->info_label = gtk_label_new ("");
    gtk_widget_add_css_class (self->info_label, "vdl-download-info");
    gtk_widget_set_hexpand (self->info_label, TRUE);
    gtk_widget_set_halign (self->info_label, GTK_ALIGN_END);
    gtk_box_append (GTK_BOX (bottom_row), self->info_label);

    gtk_box_append (GTK_BOX (self), bottom_row);

    return GTK_WIDGET (self);
}

const char *
vdl_download_row_get_download_id (VdlDownloadRow *self)
{
    return self->download_id;
}

void
vdl_download_row_update_progress (VdlDownloadRow *self,
                                   int64_t downloaded, int64_t total,
                                   double speed)
{
    g_return_if_fail (VDL_IS_DOWNLOAD_ROW (self));

    double fraction = 0;
    if (total > 0)
        fraction = (double)downloaded / (double)total;
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (self->progress_bar),
                                   CLAMP(fraction, 0.0, 1.0));

    if (!self->is_paused) {
        g_autofree char *speed_str = vdl_utils_format_speed (speed);
        gtk_label_set_text (GTK_LABEL (self->speed_label), speed_str);
    }

    g_autofree char *dl_str = vdl_utils_format_size (downloaded);
    if (total > 0) {
        g_autofree char *total_str = vdl_utils_format_size (total);
        int eta = -1;
        if (speed > 0)
            eta = (int)((total - downloaded) / speed);
        if (eta > 0) {
            g_autofree char *eta_str = vdl_utils_format_time (eta);
            g_autofree char *info = g_strdup_printf ("%s / %s  •  %s left",
                                                      dl_str, total_str, eta_str);
            gtk_label_set_text (GTK_LABEL (self->info_label), info);
        } else {
            g_autofree char *info = g_strdup_printf ("%s / %s", dl_str, total_str);
            gtk_label_set_text (GTK_LABEL (self->info_label), info);
        }
    } else {
        g_autofree char *info = g_strdup_printf ("%s downloaded", dl_str);
        gtk_label_set_text (GTK_LABEL (self->info_label), info);
    }
}

void
vdl_download_row_set_status (VdlDownloadRow *self, VdlDownloadStatus status)
{
    g_return_if_fail (VDL_IS_DOWNLOAD_ROW (self));

    gtk_widget_remove_css_class (self->status_badge, status_to_css_class (self->status));
    self->status = status;
    gtk_label_set_text (GTK_LABEL (self->status_badge), status_to_string (status));
    gtk_widget_add_css_class (self->status_badge, status_to_css_class (status));

    /* Progress bar style */
    gtk_widget_remove_css_class (self->progress_bar, "completed");
    gtk_widget_remove_css_class (self->progress_bar, "error");
    gtk_widget_remove_css_class (self->progress_bar, "seeding");

    if (status == VDL_DOWNLOAD_STATUS_COMPLETED)
        gtk_widget_add_css_class (self->progress_bar, "completed");
    else if (status == VDL_DOWNLOAD_STATUS_ERROR)
        gtk_widget_add_css_class (self->progress_bar, "error");
    else if (status == VDL_DOWNLOAD_STATUS_SEEDING)
        gtk_widget_add_css_class (self->progress_bar, "seeding");

    /* Pause state — toggle icon */
    if (status == VDL_DOWNLOAD_STATUS_PAUSED) {
        self->is_paused = TRUE;
        gtk_button_set_icon_name (GTK_BUTTON (self->pause_btn),
                                  "media-playback-start-symbolic");
        gtk_widget_set_tooltip_text (self->pause_btn, "Resume");
        gtk_label_set_text (GTK_LABEL (self->speed_label), "⏸ Paused");
    } else if (status == VDL_DOWNLOAD_STATUS_DOWNLOADING) {
        self->is_paused = FALSE;
        gtk_button_set_icon_name (GTK_BUTTON (self->pause_btn),
                                  "media-playback-pause-symbolic");
        gtk_widget_set_tooltip_text (self->pause_btn, "Pause");
    }

    /* On completion/error — swap buttons */
    if (status == VDL_DOWNLOAD_STATUS_COMPLETED ||
        status == VDL_DOWNLOAD_STATUS_ERROR) {
        gtk_widget_set_visible (self->pause_btn, FALSE);
        gtk_widget_set_visible (self->cancel_btn, FALSE);
        gtk_widget_set_visible (self->folder_btn,
            status == VDL_DOWNLOAD_STATUS_COMPLETED);

        gtk_widget_remove_css_class (self->speed_label, "vdl-speed-done");
        gtk_widget_remove_css_class (self->speed_label, "vdl-speed-error");

        if (status == VDL_DOWNLOAD_STATUS_COMPLETED) {
            gtk_label_set_text (GTK_LABEL (self->speed_label), "✓ Done");
            gtk_widget_add_css_class (self->speed_label, "vdl-speed-done");
        } else {
            gtk_label_set_text (GTK_LABEL (self->speed_label), "✗ Failed");
            gtk_widget_add_css_class (self->speed_label, "vdl-speed-error");
        }
    } else {
        gtk_widget_set_visible (self->pause_btn, TRUE);
        gtk_widget_set_visible (self->cancel_btn, TRUE);
        gtk_widget_set_visible (self->folder_btn, FALSE);
    }
}
