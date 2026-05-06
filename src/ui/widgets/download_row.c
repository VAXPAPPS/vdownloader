#include "ui/widgets/download_row.h"
#include "core/utils.h"
#include <gtk/gtk.h>

struct _DownloadRow {
    GtkBox       parent;
    gchar       *gid;
    GtkLabel    *lbl_name;
    GtkLabel    *lbl_status;
    GtkLabel    *lbl_size;
    GtkLabel    *lbl_speed;
    GtkLabel    *lbl_eta;
    GtkProgressBar *progbar;
    GtkImage    *icon;
    GtkButton   *btn_pause;
    GtkButton   *btn_remove;
    DownloadState current_state;
};

enum { SIG_ACTION_PAUSE, SIG_ACTION_REMOVE, N_SIGS };
static guint sigs[N_SIGS];

G_DEFINE_FINAL_TYPE (DownloadRow, download_row, GTK_TYPE_BOX)

static void download_row_finalize (GObject *obj)
{
    g_free (VXAP_DOWNLOAD_ROW(obj)->gid);
    G_OBJECT_CLASS (download_row_parent_class)->finalize (obj);
}

static void download_row_class_init (DownloadRowClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = download_row_finalize;
    sigs[SIG_ACTION_PAUSE] = g_signal_new ("action-pause",
        G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
        NULL, NULL, NULL, G_TYPE_NONE, 0);
    sigs[SIG_ACTION_REMOVE] = g_signal_new ("action-remove",
        G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
        NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void download_row_init (DownloadRow *self) { (void)self; }

static void _on_pause_clicked (GtkButton *b, gpointer data) {
    g_signal_emit (data, sigs[SIG_ACTION_PAUSE], 0);
}
static void _on_remove_clicked (GtkButton *b, gpointer data) {
    g_signal_emit (data, sigs[SIG_ACTION_REMOVE], 0);
}

GtkWidget *
download_row_new (DownloadItem *item)
{
    DownloadRow *self = g_object_new (VXAP_TYPE_DOWNLOAD_ROW,
                                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                                      "spacing", 12,
                                      "margin-start", 12,
                                      "margin-end", 12,
                                      "margin-top", 8,
                                      "margin-bottom", 8,
                                      NULL);
    self->gid = g_strdup (item->gid);
    gtk_widget_add_css_class (GTK_WIDGET(self), "download-row");

    /* File icon */
    self->icon = GTK_IMAGE (gtk_image_new_from_icon_name ("document-save-symbolic"));
    gtk_image_set_pixel_size (self->icon, 32);
    gtk_box_append (GTK_BOX(self), GTK_WIDGET(self->icon));

    /* Center info column */
    GtkBox *center = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 4));
    gtk_widget_set_hexpand (GTK_WIDGET(center), TRUE);

    self->lbl_name = GTK_LABEL (gtk_label_new (item->name));
    gtk_label_set_ellipsize (self->lbl_name, PANGO_ELLIPSIZE_END);
    gtk_label_set_xalign (self->lbl_name, 0);
    gtk_widget_add_css_class (GTK_WIDGET(self->lbl_name), "download-name");
    gtk_box_append (center, GTK_WIDGET(self->lbl_name));

    self->progbar = GTK_PROGRESS_BAR (gtk_progress_bar_new ());
    gtk_progress_bar_set_fraction (self->progbar, item->progress);
    gtk_widget_add_css_class (GTK_WIDGET(self->progbar), "download-progress");
    gtk_box_append (center, GTK_WIDGET(self->progbar));

    GtkBox *info = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 16));
    self->lbl_status = GTK_LABEL (gtk_label_new (""));
    self->lbl_size   = GTK_LABEL (gtk_label_new (""));
    self->lbl_speed  = GTK_LABEL (gtk_label_new (""));
    self->lbl_eta    = GTK_LABEL (gtk_label_new (""));
    GtkLabel *meta_labels[] = {self->lbl_status, self->lbl_size, self->lbl_speed, self->lbl_eta, NULL};
    for (int i = 0; meta_labels[i]; i++) {
        gtk_widget_add_css_class (GTK_WIDGET(meta_labels[i]), "download-meta");
        gtk_box_append (info, GTK_WIDGET(meta_labels[i]));
    }
    gtk_box_append (center, GTK_WIDGET(info));
    gtk_box_append (GTK_BOX(self), GTK_WIDGET(center));

    /* Action buttons */
    GtkBox *btn_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4));
    self->btn_pause  = GTK_BUTTON (gtk_button_new_from_icon_name ("media-playback-pause-symbolic"));
    self->btn_remove = GTK_BUTTON (gtk_button_new_from_icon_name ("edit-delete-symbolic"));
    gtk_widget_add_css_class (GTK_WIDGET(self->btn_pause),  "circular");
    gtk_widget_add_css_class (GTK_WIDGET(self->btn_remove), "circular");
    gtk_widget_add_css_class (GTK_WIDGET(self->btn_remove), "destructive-action");
    gtk_box_append (btn_box, GTK_WIDGET(self->btn_pause));
    gtk_box_append (btn_box, GTK_WIDGET(self->btn_remove));
    gtk_box_append (GTK_BOX(self), GTK_WIDGET(btn_box));

    g_signal_connect (self->btn_pause, "clicked", G_CALLBACK(_on_pause_clicked), self);
    g_signal_connect (self->btn_remove, "clicked", G_CALLBACK(_on_remove_clicked), self);

    download_row_update (self, item);
    return GTK_WIDGET(self);
}

void
download_row_update (DownloadRow *self, DownloadItem *item)
{
    gtk_label_set_text (self->lbl_name, item->name ? item->name : item->gid);
    gtk_progress_bar_set_fraction (self->progbar, CLAMP(item->progress, 0.0, 1.0));

    gchar *size_str = utils_format_size (item->total_length);
    gchar *done_str = utils_format_size (item->completed_length);
    gchar *size_label = g_strdup_printf ("%s / %s", done_str, size_str);
    gtk_label_set_text (self->lbl_size, size_label);
    g_free (size_str); g_free (done_str); g_free (size_label);

    if (item->state == DOWNLOAD_STATE_ACTIVE) {
        gchar *speed = utils_format_speed (item->download_speed);
        gtk_label_set_text (self->lbl_speed, speed);
        g_free (speed);
        gint64 remaining = item->total_length - item->completed_length;
        gchar *eta = utils_format_eta (remaining, item->download_speed);
        gtk_label_set_text (self->lbl_eta, eta);
        g_free (eta);
    } else {
        gtk_label_set_text (self->lbl_speed, "");
        gtk_label_set_text (self->lbl_eta, "");
    }

    static const gchar *state_icons[] = {
        "network-transmit-symbolic", "content-loading-symbolic",
        "media-playback-pause-symbolic", "emblem-ok-symbolic",
        "dialog-error-symbolic", "edit-delete-symbolic"
    };
    if ((guint)item->state < 6)
        gtk_image_set_from_icon_name (self->icon, state_icons[item->state]);

    static const gchar *state_names[] = {"Active", "Waiting", "Paused", "Complete", "Error", "Removed"};
    if ((guint)item->state < 6)
        gtk_label_set_text (self->lbl_status, state_names[item->state]);

    if (item->state == DOWNLOAD_STATE_PAUSED)
        gtk_button_set_icon_name (self->btn_pause, "media-playback-start-symbolic");
    else
        gtk_button_set_icon_name (self->btn_pause, "media-playback-pause-symbolic");

    if (item->state == DOWNLOAD_STATE_COMPLETE || 
        item->state == DOWNLOAD_STATE_ERROR || 
        item->state == DOWNLOAD_STATE_REMOVED) {
        gtk_widget_set_sensitive (GTK_WIDGET(self->btn_pause), FALSE);
    } else {
        gtk_widget_set_sensitive (GTK_WIDGET(self->btn_pause), TRUE);
    }

    /* CSS state classes */
    static const gchar *all_states[] = {"state-active","state-paused","state-complete","state-error",NULL};
    for (int i = 0; all_states[i]; i++)
        gtk_widget_remove_css_class (GTK_WIDGET(self), all_states[i]);

    switch (item->state) {
    case DOWNLOAD_STATE_ACTIVE:   gtk_widget_add_css_class (GTK_WIDGET(self), "state-active");   break;
    case DOWNLOAD_STATE_PAUSED:   gtk_widget_add_css_class (GTK_WIDGET(self), "state-paused");   break;
    case DOWNLOAD_STATE_COMPLETE: gtk_widget_add_css_class (GTK_WIDGET(self), "state-complete"); break;
    case DOWNLOAD_STATE_ERROR:    gtk_widget_add_css_class (GTK_WIDGET(self), "state-error");    break;
    default: break;
    }

    self->current_state = item->state;
}

const gchar *
download_row_get_gid (DownloadRow *self)
{
    return self->gid;
}

gboolean
download_row_is_paused (DownloadRow *self)
{
    return self->current_state == DOWNLOAD_STATE_PAUSED;
}
