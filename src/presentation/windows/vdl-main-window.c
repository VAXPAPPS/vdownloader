/*
 * VXAPDownloader - Main Window
 * Primary window: sidebar, header bar, content stack,
 * download management with pause/cancel/resume,
 * clipboard monitoring, and dashboard stats.
 */

#include "vdl-main-window.h"
#include "../../core/vdl-application.h"
#include "../../core/vdl-logger.h"
#include "../../core/vdl-config.h"
#include "../../core/vdl-utils.h"
#include "../widgets/vdl-headerbar.h"
#include "../widgets/vdl-sidebar.h"
#include "../widgets/vdl-speed-graph.h"
#include "../views/vdl-downloads-view.h"
#include "../views/vdl-categories-view.h"
#include "../views/vdl-statistics-view.h"
#include "../views/vdl-torrents-view.h"
#include "../dialogs/vdl-add-download-dialog.h"
#include "../../domain/entities/vdl-download-item.h"
#include "../../engine/http/vdl-http-engine.h"
#include "../../engine/torrent/vdl-torrent-engine.h"

struct _VdlMainWindow {
    AdwApplicationWindow  parent_instance;

    VdlApplication       *app;
    VdlHeaderBar         *headerbar;
    VdlSidebar           *sidebar;
    GtkStack             *content_stack;

    /* Views */
    GtkWidget            *dashboard_view;
    VdlDownloadsView     *downloads_view;
    VdlTorrentsView      *torrents_view;
    GtkWidget            *active_view;
    GtkWidget            *completed_view;
    GtkWidget            *statistics_view;
    GtkWidget            *settings_view;

    /* Dashboard widgets */
    GtkWidget            *active_count_label;
    GtkWidget            *completed_count_label;
    GtkWidget            *dl_speed_label;
    GtkWidget            *ul_speed_label;
    VdlSpeedGraph        *speed_graph;

    /* Stats */
    int                   active_count;
    int                   completed_count;
    int                   total_downloads;

    /* Items reference for pause/cancel */
    GHashTable           *items; /* id -> VdlDownloadItem */

    /* Clipboard monitoring */
    guint                 clipboard_timer_id;
    char                 *last_clipboard_url;

    /* Torrent engine */
    VdlTorrentEngine     *torrent_engine;
    guint                 torrent_poll_id;
};

G_DEFINE_TYPE (VdlMainWindow, vdl_main_window, ADW_TYPE_APPLICATION_WINDOW)

/* ── Dashboard ── */
static GtkWidget *
create_stat_card (const char *icon_name, const char *value,
                  const char *label, GtkWidget **value_widget)
{
    GtkWidget *card = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class (card, "vdl-stat-card");
    gtk_widget_set_hexpand (card, TRUE);

    GtkWidget *icon = gtk_image_new_from_icon_name (icon_name);
    gtk_image_set_pixel_size (GTK_IMAGE (icon), 28);
    gtk_widget_add_css_class (icon, "vdl-stat-icon");
    gtk_widget_set_halign (icon, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (card), icon);

    GtkWidget *val = gtk_label_new (value);
    gtk_widget_add_css_class (val, "vdl-stat-value");
    gtk_widget_set_halign (val, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (card), val);
    if (value_widget) *value_widget = val;

    GtkWidget *lbl = gtk_label_new (label);
    gtk_widget_add_css_class (lbl, "vdl-stat-label");
    gtk_widget_set_halign (lbl, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (card), lbl);

    return card;
}

static GtkWidget *
create_dashboard_view (VdlMainWindow *self)
{
    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class (vbox, "vdl-content");

    GtkWidget *header = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class (header, "vdl-content-header");
    GtkWidget *title = gtk_label_new ("Dashboard");
    gtk_widget_add_css_class (title, "vdl-content-title");
    gtk_widget_set_halign (title, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (header), title);
    GtkWidget *subtitle = gtk_label_new ("Overview of your downloads");
    gtk_widget_add_css_class (subtitle, "vdl-content-subtitle");
    gtk_widget_set_halign (subtitle, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (header), subtitle);
    gtk_box_append (GTK_BOX (vbox), header);

    /* Stats cards */
    GtkWidget *cards = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_margin_start (cards, 24);
    gtk_widget_set_margin_end (cards, 24);
    gtk_widget_set_margin_top (cards, 20);

    gtk_box_append (GTK_BOX (cards),
        create_stat_card ("folder-download-symbolic", "0", "ACTIVE", &self->active_count_label));
    gtk_box_append (GTK_BOX (cards),
        create_stat_card ("emblem-ok-symbolic", "0", "COMPLETED", &self->completed_count_label));
    gtk_box_append (GTK_BOX (cards),
        create_stat_card ("go-down-symbolic", "0 B/s", "DOWNLOAD", &self->dl_speed_label));
    gtk_box_append (GTK_BOX (cards),
        create_stat_card ("go-up-symbolic", "0 B/s", "UPLOAD", &self->ul_speed_label));
    gtk_box_append (GTK_BOX (vbox), cards);

    /* Speed graph */
    GtkWidget *graph_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class (graph_box, "vdl-speed-graph");
    gtk_widget_set_margin_start (graph_box, 24);
    gtk_widget_set_margin_end (graph_box, 24);
    gtk_widget_set_margin_top (graph_box, 20);
    gtk_widget_set_vexpand (graph_box, TRUE);

    GtkWidget *gt = gtk_label_new ("Download Speed");
    gtk_widget_add_css_class (gt, "vdl-graph-title");
    gtk_widget_set_halign (gt, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (graph_box), gt);

    self->speed_graph = VDL_SPEED_GRAPH (vdl_speed_graph_new ());
    gtk_widget_set_vexpand (GTK_WIDGET (self->speed_graph), TRUE);
    gtk_box_append (GTK_BOX (graph_box), GTK_WIDGET (self->speed_graph));

    gtk_box_append (GTK_BOX (vbox), graph_box);
    return vbox;
}

/* ── Settings View ── */
static GtkWidget *
create_settings_view (VdlMainWindow *self)
{
    VdlConfig *config = vdl_application_get_config (self->app);
    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class (vbox, "vdl-content");

    GtkWidget *header = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class (header, "vdl-content-header");
    GtkWidget *title = gtk_label_new ("Settings");
    gtk_widget_add_css_class (title, "vdl-content-title");
    gtk_widget_set_halign (title, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (header), title);
    GtkWidget *sub = gtk_label_new ("Application preferences");
    gtk_widget_add_css_class (sub, "vdl-content-subtitle");
    gtk_widget_set_halign (sub, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (header), sub);
    gtk_box_append (GTK_BOX (vbox), header);

    GtkWidget *scroll = gtk_scrolled_window_new ();
    gtk_widget_set_vexpand (scroll, TRUE);
    GtkWidget *settings_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start (settings_box, 28);
    gtk_widget_set_margin_end (settings_box, 28);
    gtk_widget_set_margin_top (settings_box, 20);

    /* Download settings group */
    GtkWidget *dl_group = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class (dl_group, "vdl-pref-group");
    GtkWidget *dl_title = gtk_label_new ("Download Settings");
    gtk_widget_add_css_class (dl_title, "vdl-pref-title");
    gtk_widget_set_halign (dl_title, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (dl_group), dl_title);

    /* Download directory */
    GtkWidget *row1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class (row1, "vdl-pref-row");
    GtkWidget *label1 = gtk_label_new ("Download Directory");
    gtk_widget_set_hexpand (label1, TRUE);
    gtk_widget_set_halign (label1, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (row1), label1);
    const char *dl_dir = config ? vdl_config_get_download_dir (config) : "~/Downloads";
    GtkWidget *dir_label = gtk_label_new (dl_dir);
    gtk_widget_set_opacity (dir_label, 0.6);
    gtk_box_append (GTK_BOX (row1), dir_label);
    gtk_box_append (GTK_BOX (dl_group), row1);

    /* Max concurrent */
    GtkWidget *row2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class (row2, "vdl-pref-row");
    GtkWidget *label2 = gtk_label_new ("Max Concurrent Downloads");
    gtk_widget_set_hexpand (label2, TRUE);
    gtk_widget_set_halign (label2, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (row2), label2);
    GtkWidget *spin1 = gtk_spin_button_new_with_range (1, 16, 1);
    int max_dl = config ? vdl_config_get_max_concurrent (config) : 4;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin1), max_dl);
    gtk_box_append (GTK_BOX (row2), spin1);
    gtk_box_append (GTK_BOX (dl_group), row2);

    /* Default segments */
    GtkWidget *row3 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class (row3, "vdl-pref-row");
    GtkWidget *label3 = gtk_label_new ("Default Segments per Download");
    gtk_widget_set_hexpand (label3, TRUE);
    gtk_widget_set_halign (label3, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (row3), label3);
    GtkWidget *spin2 = gtk_spin_button_new_with_range (1, 32, 1);
    int segs = config ? vdl_config_get_segments (config) : 8;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin2), segs);
    gtk_box_append (GTK_BOX (row3), spin2);
    gtk_box_append (GTK_BOX (dl_group), row3);

    gtk_box_append (GTK_BOX (settings_box), dl_group);

    /* Speed settings group */
    GtkWidget *sp_group = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class (sp_group, "vdl-pref-group");
    GtkWidget *sp_title = gtk_label_new ("Speed & Monitoring");
    gtk_widget_add_css_class (sp_title, "vdl-pref-title");
    gtk_widget_set_halign (sp_title, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (sp_group), sp_title);

    /* Speed limit */
    GtkWidget *row4 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class (row4, "vdl-pref-row");
    GtkWidget *label4 = gtk_label_new ("Speed Limit (KB/s, 0=unlimited)");
    gtk_widget_set_hexpand (label4, TRUE);
    gtk_widget_set_halign (label4, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (row4), label4);
    GtkWidget *spin3 = gtk_spin_button_new_with_range (0, 102400, 100);
    int64_t lim = config ? vdl_config_get_speed_limit (config) : 0;
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin3), (double)(lim / 1024));
    gtk_box_append (GTK_BOX (row4), spin3);
    gtk_box_append (GTK_BOX (sp_group), row4);

    /* Clipboard watch */
    GtkWidget *row5 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class (row5, "vdl-pref-row");
    GtkWidget *label5 = gtk_label_new ("Watch Clipboard for URLs");
    gtk_widget_set_hexpand (label5, TRUE);
    gtk_widget_set_halign (label5, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (row5), label5);
    GtkWidget *toggle = gtk_switch_new ();
    gboolean watch = config ? vdl_config_get_clipboard_watch (config) : TRUE;
    gtk_switch_set_active (GTK_SWITCH (toggle), watch);
    gtk_widget_set_valign (toggle, GTK_ALIGN_CENTER);
    gtk_box_append (GTK_BOX (row5), toggle);
    gtk_box_append (GTK_BOX (sp_group), row5);

    gtk_box_append (GTK_BOX (settings_box), sp_group);

    /* About section */
    GtkWidget *about_group = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class (about_group, "vdl-pref-group");
    GtkWidget *about_title = gtk_label_new ("About");
    gtk_widget_add_css_class (about_title, "vdl-pref-title");
    gtk_widget_set_halign (about_title, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (about_group), about_title);
    GtkWidget *about_info = gtk_label_new (
        "VXAP Downloader v1.0.0\n"
        "Part of the VAXP-OS Ecosystem\n\n"
        "© 2026 VAXP Team. All rights reserved.");
    gtk_widget_set_opacity (about_info, 0.6);
    gtk_widget_set_halign (about_info, GTK_ALIGN_START);
    gtk_label_set_wrap (GTK_LABEL (about_info), TRUE);
    gtk_box_append (GTK_BOX (about_group), about_info);
    gtk_box_append (GTK_BOX (settings_box), about_group);

    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroll), settings_box);
    gtk_box_append (GTK_BOX (vbox), scroll);
    return vbox;
}

/* ── Placeholder ── */
static GtkWidget *
create_placeholder_view (const char *t, const char *s, const char *ic)
{
    GtkWidget *v = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class (v, "vdl-content");
    GtkWidget *h = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class (h, "vdl-content-header");
    GtkWidget *tl = gtk_label_new (t);
    gtk_widget_add_css_class (tl, "vdl-content-title");
    gtk_widget_set_halign (tl, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (h), tl);
    GtkWidget *sl = gtk_label_new (s);
    gtk_widget_add_css_class (sl, "vdl-content-subtitle");
    gtk_widget_set_halign (sl, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (h), sl);
    gtk_box_append (GTK_BOX (v), h);
    GtkWidget *c = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_vexpand (c, TRUE);
    gtk_widget_set_valign (c, GTK_ALIGN_CENTER);
    gtk_widget_set_halign (c, GTK_ALIGN_CENTER);
    GtkWidget *i = gtk_image_new_from_icon_name (ic);
    gtk_image_set_pixel_size (GTK_IMAGE (i), 64);
    gtk_widget_set_opacity (i, 0.3);
    gtk_box_append (GTK_BOX (c), i);
    GtkWidget *l = gtk_label_new ("No items");
    gtk_widget_set_opacity (l, 0.4);
    gtk_box_append (GTK_BOX (c), l);
    gtk_box_append (GTK_BOX (v), c);
    return v;
}

/* ── View Switching ── */
static void
on_view_changed (VdlSidebar *sidebar, int view, gpointer user_data)
{
    (void)sidebar;
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);
    const char *name = NULL;
    switch (view) {
        case VDL_VIEW_DASHBOARD:      name = "dashboard"; break;
        case VDL_VIEW_ALL_DOWNLOADS:  name = "all-downloads"; break;
        case VDL_VIEW_ACTIVE:         name = "active"; break;
        case VDL_VIEW_COMPLETED:      name = "completed"; break;
        case VDL_VIEW_TORRENTS:       name = "torrents"; break;
        case VDL_VIEW_STATISTICS:     name = "statistics"; break;
        case VDL_VIEW_SETTINGS:       name = "settings"; break;
        default:                      name = "dashboard"; break;
    }
    gtk_stack_set_visible_child_name (self->content_stack, name);
}

/* ── Download item progress tracking for dashboard ── */
static void
on_item_progress (VdlDownloadItem *item, gint64 downloaded, gint64 total,
                  gdouble speed, gpointer user_data)
{
    (void)downloaded; (void)total;
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);

    /* Update dashboard speed */
    g_autofree char *speed_str = vdl_utils_format_speed (speed);
    gtk_label_set_text (GTK_LABEL (self->dl_speed_label), speed_str);

    /* Feed speed graph */
    vdl_speed_graph_add_sample (self->speed_graph, speed, 0);

    /* Update counts on status change */
    VdlDownloadStatus st = vdl_download_item_get_status (item);
    if (st == VDL_DOWNLOAD_STATUS_COMPLETED) {
        g_autofree char *c = g_strdup_printf ("%d", ++self->completed_count);
        gtk_label_set_text (GTK_LABEL (self->completed_count_label), c);
    }
}

/* ── Download Requested ── */
static void
on_download_requested (VdlAddDownloadDialog *dialog, VdlDownloadItem *item,
                       gpointer user_data)
{
    (void)dialog;
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);
    const char *id = vdl_download_item_get_id (item);

    /* Store item reference */
    g_hash_table_insert (self->items, g_strdup (id), g_object_ref (item));
    self->total_downloads++;
    self->active_count++;
    g_autofree char *ac = g_strdup_printf ("%d", self->active_count);
    gtk_label_set_text (GTK_LABEL (self->active_count_label), ac);

    /* Track progress for dashboard */
    g_signal_connect (item, "progress-changed",
                      G_CALLBACK (on_item_progress), self);

    /* Add to view */
    vdl_downloads_view_add_item (self->downloads_view, item);
    gtk_stack_set_visible_child_name (self->content_stack, "all-downloads");
    vdl_sidebar_set_active (self->sidebar, VDL_VIEW_ALL_DOWNLOADS);

    /* Start download */
    VdlHttpEngine *engine = vdl_application_get_http_engine (self->app);
    if (engine)
        vdl_http_engine_start_download (engine, item);

    vdl_logger_info ("Download %d started: %s",
                     self->total_downloads,
                     vdl_download_item_get_filename (item));
}

/* ── Pause/Cancel handlers ── */
static void
on_pause_download (VdlDownloadsView *view, const char *id, gpointer user_data)
{
    (void)view;
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);
    VdlHttpEngine *engine = vdl_application_get_http_engine (self->app);
    VdlDownloadItem *item = g_hash_table_lookup (self->items, id);
    if (!engine || !item) return;

    VdlDownloadStatus st = vdl_download_item_get_status (item);
    if (st == VDL_DOWNLOAD_STATUS_PAUSED) {
        vdl_http_engine_resume_download (engine, id);
        vdl_download_item_set_status (item, VDL_DOWNLOAD_STATUS_DOWNLOADING);
        vdl_logger_info ("Resumed: %s", id);
    } else {
        vdl_http_engine_pause_download (engine, id);
        vdl_download_item_set_status (item, VDL_DOWNLOAD_STATUS_PAUSED);
        vdl_logger_info ("Paused: %s", id);
    }
    /* Trigger UI update */
    g_signal_emit_by_name (item, "progress-changed",
                           vdl_download_item_get_downloaded (item),
                           vdl_download_item_get_total_size (item),
                           0.0);
}

static void
on_cancel_download (VdlDownloadsView *view, const char *id, gpointer user_data)
{
    (void)view;
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);
    VdlHttpEngine *engine = vdl_application_get_http_engine (self->app);
    if (engine)
        vdl_http_engine_cancel_download (engine, id);
    self->active_count = MAX(0, self->active_count - 1);
    g_autofree char *ac = g_strdup_printf ("%d", self->active_count);
    gtk_label_set_text (GTK_LABEL (self->active_count_label), ac);
}

/* ── Add Download ── */
static void
on_add_download_clicked (GtkButton *btn, gpointer user_data)
{
    (void)btn;
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);
    vdl_main_window_show_add_dialog (self, NULL);
}

void
vdl_main_window_show_add_dialog (VdlMainWindow *self, const char *url)
{
    g_return_if_fail (VDL_IS_MAIN_WINDOW (self));

    VdlAddDownloadDialog *dialog = vdl_add_download_dialog_new (GTK_WINDOW (self));
    
    if (url) {
        /* Set the URL in the dialog's entry if provided */
        GtkWidget *entry = g_object_get_data (G_OBJECT (dialog), "url_entry");
        if (entry) {
            gtk_editable_set_text (GTK_EDITABLE (entry), url);
        }
    }

    g_signal_connect (dialog, "download-requested",
                      G_CALLBACK (on_download_requested), self);
    gtk_window_present (GTK_WINDOW (dialog));
}

/* ── Clipboard Monitoring ── */
static void
on_clipboard_text_ready (GObject *source, GAsyncResult *result, gpointer user_data)
{
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);
    GdkClipboard *clipboard = GDK_CLIPBOARD (source);
    g_autofree char *text = gdk_clipboard_read_text_finish (clipboard, result, NULL);

    if (!text || !*text) return;

    /* Check if it's a valid download URL and different from last */
    if (vdl_utils_is_valid_url (text) &&
        g_strcmp0 (text, self->last_clipboard_url) != 0) {

        g_free (self->last_clipboard_url);
        self->last_clipboard_url = g_strdup (text);

        vdl_logger_info ("Clipboard URL detected: %s", text);

        /* Auto-open add download dialog with the URL */
        VdlAddDownloadDialog *dialog = vdl_add_download_dialog_new (GTK_WINDOW (self));
        g_signal_connect (dialog, "download-requested",
                          G_CALLBACK (on_download_requested), self);
        gtk_window_present (GTK_WINDOW (dialog));
    }
}

static gboolean
clipboard_check_tick (gpointer user_data)
{
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);
    GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (self));
    GdkClipboard *clipboard = gdk_display_get_clipboard (display);

    gdk_clipboard_read_text_async (clipboard, NULL,
                                    on_clipboard_text_ready, self);
    return G_SOURCE_CONTINUE;
}

/* ── Torrent: file chooser response ── */
static void
on_torrent_file_chosen (GObject *source, GAsyncResult *result, gpointer user_data)
{
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);
    GtkFileDialog *dialog = GTK_FILE_DIALOG (source);
    GFile *file = gtk_file_dialog_open_finish (dialog, result, NULL);
    if (!file) return;

    g_autofree char *path = g_file_get_path (file);
    g_object_unref (file);

    VdlTorrentEngine *te = vdl_application_get_torrent_engine (self->app);
    if (!te || !path) return;

    VdlConfig *config = vdl_application_get_config (self->app);
    const char *save = config ? vdl_config_get_download_dir (config) : NULL;

    int idx = vdl_torrent_engine_add_torrent_file (te, path, save);
    if (idx >= 0) {
        VdlTorrentInfo info;
        if (vdl_torrent_engine_get_info (te, idx, &info))
            vdl_torrents_view_add_torrent (self->torrents_view, info.name, idx);
        else
            vdl_torrents_view_add_torrent (self->torrents_view, path, idx);
        gtk_stack_set_visible_child_name (self->content_stack, "torrents");
        vdl_sidebar_set_active (self->sidebar, VDL_VIEW_TORRENTS);
    }
}

static void
on_add_torrent_clicked (VdlTorrentsView *view, gpointer user_data)
{
    (void)view;
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);

    GtkFileDialog *dialog = gtk_file_dialog_new ();
    gtk_file_dialog_set_title (dialog, "Select Torrent File");
    GtkFileFilter *filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, "Torrent files");
    gtk_file_filter_add_pattern (filter, "*.torrent");
    GListStore *filters = g_list_store_new (GTK_TYPE_FILE_FILTER);
    g_list_store_append (filters, filter);
    gtk_file_dialog_set_filters (dialog, G_LIST_MODEL (filters));

    gtk_file_dialog_open (dialog, GTK_WINDOW (self), NULL,
                          on_torrent_file_chosen, self);
    g_object_unref (filter);
    g_object_unref (filters);
}

/* ── Torrent poll timer: update all torrent rows ── */
static gboolean
torrent_poll (gpointer user_data)
{
    VdlMainWindow *self = VDL_MAIN_WINDOW (user_data);
    VdlTorrentEngine *te = vdl_application_get_torrent_engine (self->app);
    if (!te) return G_SOURCE_CONTINUE;

    int count = vdl_torrent_engine_count (te);
    for (int i = 0; i < count; i++) {
        VdlTorrentInfo info;
        if (vdl_torrent_engine_get_info (te, i, &info)) {
            const char *status_text = "Downloading";
            switch (info.status) {
                case VDL_TORRENT_CHECKING:         status_text = "Checking"; break;
                case VDL_TORRENT_DOWNLOADING_META:  status_text = "Metadata"; break;
                case VDL_TORRENT_DOWNLOADING:       status_text = "Downloading"; break;
                case VDL_TORRENT_FINISHED:          status_text = "Finished"; break;
                case VDL_TORRENT_SEEDING:           status_text = "Seeding"; break;
                case VDL_TORRENT_PAUSED:            status_text = "Paused"; break;
                case VDL_TORRENT_ERROR:             status_text = "Error"; break;
            }
            vdl_torrents_view_update_torrent (self->torrents_view, i,
                info.progress, info.download_rate, info.upload_rate,
                info.num_peers, info.num_seeds, info.eta_seconds,
                status_text);
        }
    }

    /* Update upload speed on dashboard */
    double ul_rate = vdl_torrent_engine_total_upload_rate (te);
    if (ul_rate > 0) {
        g_autofree char *ul_str = vdl_utils_format_speed (ul_rate);
        gtk_label_set_text (GTK_LABEL (self->ul_speed_label), ul_str);
    }

    return G_SOURCE_CONTINUE;
}

/* ── GObject ── */
static void
vdl_main_window_finalize (GObject *obj)
{
    VdlMainWindow *self = VDL_MAIN_WINDOW (obj);
    if (self->clipboard_timer_id > 0)
        g_source_remove (self->clipboard_timer_id);
    if (self->torrent_poll_id > 0)
        g_source_remove (self->torrent_poll_id);
    g_free (self->last_clipboard_url);
    g_hash_table_destroy (self->items);
    G_OBJECT_CLASS (vdl_main_window_parent_class)->finalize (obj);
}

static void
vdl_main_window_class_init (VdlMainWindowClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = vdl_main_window_finalize;
}

static void
vdl_main_window_init (VdlMainWindow *self)
{
    self->items = g_hash_table_new_full (g_str_hash, g_str_equal,
                                         g_free, g_object_unref);
    self->active_count = 0;
    self->completed_count = 0;
    self->total_downloads = 0;
    self->clipboard_timer_id = 0;
    self->last_clipboard_url = NULL;
    self->torrent_engine = NULL;
    self->torrent_poll_id = 0;
}

VdlMainWindow *
vdl_main_window_new (VdlApplication *app)
{
    VdlMainWindow *self = g_object_new (VDL_TYPE_MAIN_WINDOW,
                                        "application", app,
                                        "default-width", 1200,
                                        "default-height", 750,
                                        "title", VDL_APP_NAME,
                                        NULL);
    self->app = app;
    gtk_window_set_decorated (GTK_WINDOW (self), FALSE);
    gtk_widget_add_css_class (GTK_WIDGET (self), "vdl-main");

    /* Root */
    GtkWidget *root = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    adw_application_window_set_content (ADW_APPLICATION_WINDOW (self), root);

    /* Header */
    self->headerbar = vdl_headerbar_new (GTK_WINDOW (self));
    gtk_box_append (GTK_BOX (root), GTK_WIDGET (self->headerbar));

    GtkWidget *add_btn = gtk_button_new_with_label ("+ New Download");
    gtk_widget_add_css_class (add_btn, "vdl-btn-primary");
    g_signal_connect (add_btn, "clicked", G_CALLBACK (on_add_download_clicked), self);
    gtk_box_append (GTK_BOX (vdl_headerbar_get_start_box (self->headerbar)), add_btn);

    /* Main layout */
    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_vexpand (hbox, TRUE);
    gtk_box_append (GTK_BOX (root), hbox);

    self->sidebar = vdl_sidebar_new ();
    gtk_box_append (GTK_BOX (hbox), GTK_WIDGET (self->sidebar));
    g_signal_connect (self->sidebar, "view-changed",
                      G_CALLBACK (on_view_changed), self);

    /* Content stack */
    self->content_stack = GTK_STACK (gtk_stack_new ());
    gtk_stack_set_transition_type (self->content_stack, GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration (self->content_stack, 200);
    gtk_widget_set_hexpand (GTK_WIDGET (self->content_stack), TRUE);
    gtk_box_append (GTK_BOX (hbox), GTK_WIDGET (self->content_stack));

    /* Views */
    self->dashboard_view = create_dashboard_view (self);
    gtk_stack_add_named (self->content_stack, self->dashboard_view, "dashboard");

    self->downloads_view = VDL_DOWNLOADS_VIEW (vdl_downloads_view_new ());
    gtk_stack_add_named (self->content_stack,
                         GTK_WIDGET (self->downloads_view), "all-downloads");
    g_signal_connect (self->downloads_view, "pause-download",
                      G_CALLBACK (on_pause_download), self);
    g_signal_connect (self->downloads_view, "cancel-download",
                      G_CALLBACK (on_cancel_download), self);

    self->active_view = create_placeholder_view ("Active Downloads",
        "Currently downloading", "emblem-synchronizing-symbolic");
    gtk_stack_add_named (self->content_stack, self->active_view, "active");

    self->completed_view = create_placeholder_view ("Completed Downloads",
        "Finished downloads", "emblem-ok-symbolic");
    gtk_stack_add_named (self->content_stack, self->completed_view, "completed");

    self->torrents_view = VDL_TORRENTS_VIEW (vdl_torrents_view_new ());
    gtk_stack_add_named (self->content_stack,
                         GTK_WIDGET (self->torrents_view), "torrents");
    g_signal_connect (self->torrents_view, "add-torrent-clicked",
                      G_CALLBACK (on_add_torrent_clicked), self);

    self->statistics_view = vdl_statistics_view_new ();
    gtk_stack_add_named (self->content_stack, self->statistics_view, "statistics");

    self->settings_view = create_settings_view (self);
    gtk_stack_add_named (self->content_stack, self->settings_view, "settings");

    gtk_stack_set_visible_child_name (self->content_stack, "dashboard");

    /* Start clipboard monitoring (check every 2 seconds) */
    VdlConfig *config = vdl_application_get_config (self->app);
    gboolean watch_clipboard = config ? vdl_config_get_clipboard_watch (config) : FALSE;
    if (watch_clipboard) {
        self->clipboard_timer_id = g_timeout_add_seconds (2, clipboard_check_tick, self);
    }

    /* Start torrent poll timer (every 1 second) */
    self->torrent_engine = vdl_application_get_torrent_engine (self->app);
    self->torrent_poll_id = g_timeout_add (1000, torrent_poll, self);

    return self;
}
