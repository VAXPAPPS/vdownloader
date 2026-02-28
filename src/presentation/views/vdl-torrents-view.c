/*
 * VXAPDownloader - Torrents View
 * Displays torrents with live progress, speed, peers, seeds.
 */

#include "vdl-torrents-view.h"
#include "../../core/vdl-utils.h"

typedef struct {
    GtkWidget *row_widget;
    GtkWidget *name_label;
    GtkWidget *progress_bar;
    GtkWidget *speed_label;
    GtkWidget *peers_label;
    GtkWidget *status_badge;
    GtkWidget *info_label;
    int        index;
} TorrentRow;

struct _VdlTorrentsView {
    GtkBox       parent_instance;
    GtkWidget   *list_box;
    GtkWidget   *empty_state;
    GtkWidget   *count_label;
    GArray      *rows;
    int          torrent_count;
};

G_DEFINE_TYPE (VdlTorrentsView, vdl_torrents_view, GTK_TYPE_BOX)

enum { SIGNAL_ADD_TORRENT_CLICKED, N_SIGNALS };
static guint tv_signals[N_SIGNALS] = { 0 };

static void
on_add_torrent_btn (GtkButton *btn, gpointer user_data)
{
    (void)btn;
    g_signal_emit (user_data, tv_signals[SIGNAL_ADD_TORRENT_CLICKED], 0);
}

static void
vdl_torrents_view_finalize (GObject *obj)
{
    VdlTorrentsView *self = VDL_TORRENTS_VIEW (obj);
    g_array_free (self->rows, TRUE);
    G_OBJECT_CLASS (vdl_torrents_view_parent_class)->finalize (obj);
}

static void
vdl_torrents_view_class_init (VdlTorrentsViewClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = vdl_torrents_view_finalize;
    tv_signals[SIGNAL_ADD_TORRENT_CLICKED] = g_signal_new (
        "add-torrent-clicked", VDL_TYPE_TORRENTS_VIEW, G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void
vdl_torrents_view_init (VdlTorrentsView *self)
{
    gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
    gtk_widget_add_css_class (GTK_WIDGET (self), "vdl-content");
    self->torrent_count = 0;
    self->rows = g_array_new (FALSE, TRUE, sizeof(TorrentRow));

    /* Header */
    GtkWidget *header = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class (header, "vdl-content-header");

    GtkWidget *title_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand (title_box, TRUE);
    GtkWidget *title = gtk_label_new ("Torrents");
    gtk_widget_add_css_class (title, "vdl-content-title");
    gtk_widget_set_halign (title, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (title_box), title);
    self->count_label = gtk_label_new ("0 torrents");
    gtk_widget_add_css_class (self->count_label, "vdl-content-subtitle");
    gtk_widget_set_halign (self->count_label, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (title_box), self->count_label);
    gtk_box_append (GTK_BOX (header), title_box);

    GtkWidget *add_btn = gtk_button_new_with_label ("+ Add Torrent");
    gtk_widget_add_css_class (add_btn, "vdl-btn-primary");
    gtk_widget_set_valign (add_btn, GTK_ALIGN_CENTER);
    g_signal_connect (add_btn, "clicked", G_CALLBACK (on_add_torrent_btn), self);
    gtk_box_append (GTK_BOX (header), add_btn);
    gtk_box_append (GTK_BOX (self), header);

    /* List */
    GtkWidget *scrolled = gtk_scrolled_window_new ();
    gtk_widget_set_vexpand (scrolled, TRUE);
    self->list_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_top (self->list_box, 8);
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), self->list_box);
    gtk_box_append (GTK_BOX (self), scrolled);

    /* Empty state */
    self->empty_state = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_valign (self->empty_state, GTK_ALIGN_CENTER);
    gtk_widget_set_halign (self->empty_state, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (self->empty_state, 80);
    GtkWidget *icon = gtk_image_new_from_icon_name ("network-transmit-receive-symbolic");
    gtk_image_set_pixel_size (GTK_IMAGE (icon), 64);
    gtk_widget_set_opacity (icon, 0.3);
    gtk_box_append (GTK_BOX (self->empty_state), icon);
    GtkWidget *msg = gtk_label_new ("No torrents\nAdd a .torrent file or magnet link");
    gtk_widget_set_opacity (msg, 0.4);
    gtk_label_set_justify (GTK_LABEL (msg), GTK_JUSTIFY_CENTER);
    gtk_box_append (GTK_BOX (self->empty_state), msg);
    gtk_box_append (GTK_BOX (self->list_box), self->empty_state);
}

GtkWidget *
vdl_torrents_view_new (void)
{
    return GTK_WIDGET (g_object_new (VDL_TYPE_TORRENTS_VIEW, NULL));
}

void
vdl_torrents_view_add_torrent (VdlTorrentsView *self,
                                const char *name, int index)
{
    TorrentRow tr = { 0 };
    tr.index = index;

    tr.row_widget = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class (tr.row_widget, "vdl-download-row");

    /* Top: name + status + peers */
    GtkWidget *top = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *icon = gtk_image_new_from_icon_name ("network-transmit-receive-symbolic");
    gtk_image_set_pixel_size (GTK_IMAGE (icon), 20);
    gtk_widget_set_opacity (icon, 0.6);
    gtk_box_append (GTK_BOX (top), icon);

    tr.name_label = gtk_label_new (name);
    gtk_widget_add_css_class (tr.name_label, "vdl-download-filename");
    gtk_widget_set_hexpand (tr.name_label, TRUE);
    gtk_widget_set_halign (tr.name_label, GTK_ALIGN_START);
    gtk_label_set_ellipsize (GTK_LABEL (tr.name_label), PANGO_ELLIPSIZE_MIDDLE);
    gtk_box_append (GTK_BOX (top), tr.name_label);

    tr.status_badge = gtk_label_new ("Starting");
    gtk_widget_add_css_class (tr.status_badge, "vdl-badge");
    gtk_widget_add_css_class (tr.status_badge, "vdl-badge-downloading");
    gtk_box_append (GTK_BOX (top), tr.status_badge);

    tr.peers_label = gtk_label_new ("0 peers");
    gtk_widget_add_css_class (tr.peers_label, "vdl-download-info");
    gtk_box_append (GTK_BOX (top), tr.peers_label);

    gtk_box_append (GTK_BOX (tr.row_widget), top);

    /* Progress */
    tr.progress_bar = gtk_progress_bar_new ();
    gtk_widget_add_css_class (tr.progress_bar, "vdl-progress");
    gtk_box_append (GTK_BOX (tr.row_widget), tr.progress_bar);

    /* Bottom: speed + info */
    GtkWidget *bot = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    tr.speed_label = gtk_label_new ("↓ 0 B/s  ↑ 0 B/s");
    gtk_widget_add_css_class (tr.speed_label, "vdl-download-speed");
    gtk_box_append (GTK_BOX (bot), tr.speed_label);

    tr.info_label = gtk_label_new ("");
    gtk_widget_add_css_class (tr.info_label, "vdl-download-info");
    gtk_widget_set_hexpand (tr.info_label, TRUE);
    gtk_widget_set_halign (tr.info_label, GTK_ALIGN_END);
    gtk_box_append (GTK_BOX (bot), tr.info_label);
    gtk_box_append (GTK_BOX (tr.row_widget), bot);

    gtk_box_prepend (GTK_BOX (self->list_box), tr.row_widget);
    g_array_append_val (self->rows, tr);
    self->torrent_count++;

    gtk_widget_set_visible (self->empty_state, FALSE);
    g_autofree char *c = g_strdup_printf ("%d torrents", self->torrent_count);
    gtk_label_set_text (GTK_LABEL (self->count_label), c);
}

void
vdl_torrents_view_update_torrent (VdlTorrentsView *self, int index,
                                   double progress, double dl_speed,
                                   double ul_speed, int peers,
                                   int seeds, int eta,
                                   const char *status_text)
{
    for (guint i = 0; i < self->rows->len; i++) {
        TorrentRow *tr = &g_array_index (self->rows, TorrentRow, i);
        if (tr->index != index) continue;

        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (tr->progress_bar),
                                       CLAMP(progress, 0, 1));

        g_autofree char *dl = vdl_utils_format_speed (dl_speed);
        g_autofree char *ul = vdl_utils_format_speed (ul_speed);
        g_autofree char *sp = g_strdup_printf ("↓ %s  ↑ %s", dl, ul);
        gtk_label_set_text (GTK_LABEL (tr->speed_label), sp);

        g_autofree char *p = g_strdup_printf ("%d peers · %d seeds", peers, seeds);
        gtk_label_set_text (GTK_LABEL (tr->peers_label), p);

        gtk_label_set_text (GTK_LABEL (tr->status_badge), status_text);

        if (eta > 0) {
            g_autofree char *eta_s = vdl_utils_format_time (eta);
            g_autofree char *pct = g_strdup_printf ("%.1f%%  •  %s left",
                                                     progress * 100, eta_s);
            gtk_label_set_text (GTK_LABEL (tr->info_label), pct);
        } else {
            g_autofree char *pct = g_strdup_printf ("%.1f%%", progress * 100);
            gtk_label_set_text (GTK_LABEL (tr->info_label), pct);
        }
        break;
    }
}
