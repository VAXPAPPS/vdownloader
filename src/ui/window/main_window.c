#include "ui/window/main_window.h"
#include "ui/window/add_dialog.h"
#include "ui/widgets/download_row.h"
#include "ui/widgets/sidebar.h"
#include "ui/widgets/statusbar.h"
#include "core/utils.h"
#include <gtk/gtk.h>

struct _MainWindow {
    GtkApplicationWindow parent;
};

typedef struct {
    DownloadManager *manager;
    Sidebar         *sidebar;
    Statusbar       *statbar;
    GtkListBox      *list;
    GtkSearchEntry  *search;
    GtkStack        *stack;
    gchar           *active_filter;
    GHashTable      *rows; /* gid -> DownloadRow* */
} MainWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW)

static void main_window_finalize (GObject *obj)
{
    MainWindowPrivate *p = main_window_get_instance_private (VXAP_MAIN_WINDOW(obj));
    g_free (p->active_filter);
    g_hash_table_destroy (p->rows);
    G_OBJECT_CLASS(main_window_parent_class)->finalize (obj);
}
static void main_window_class_init (MainWindowClass *k)
{ G_OBJECT_CLASS(k)->finalize = main_window_finalize; }
static void main_window_init (MainWindow *self)
{
    MainWindowPrivate *p = main_window_get_instance_private (self);
    p->active_filter = g_strdup ("all");
    p->rows = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}

/* ─── Filter function ─── */
static gboolean
_filter_func (GtkListBoxRow *row, gpointer user_data)
{
    MainWindowPrivate *p = user_data;
    GtkWidget *child = gtk_list_box_row_get_child (row);
    if (!VXAP_IS_DOWNLOAD_ROW (child)) return TRUE;
    DownloadRow *dr = VXAP_DOWNLOAD_ROW (child);

    if (g_strcmp0 (p->active_filter, "all") != 0) {
        gchar *cls = g_strdup_printf ("state-%s", p->active_filter);
        gboolean match = gtk_widget_has_css_class (GTK_WIDGET(dr), cls);
        g_free (cls);
        if (!match) return FALSE;
    }
    const gchar *query = gtk_editable_get_text (GTK_EDITABLE(p->search));
    if (query && *query) {
        const gchar *gid = download_row_get_gid (dr);
        return g_str_match_string (query, gid, TRUE);
    }
    return TRUE;
}

/* ─── Manager signal callbacks ─── */
static void _on_row_pause (DownloadRow *row, gpointer ud)
{
    MainWindowPrivate *p = ud;
    const gchar *gid = download_row_get_gid (row);
    if (download_row_is_paused (row))
        download_manager_resume (p->manager, gid);
    else
        download_manager_pause (p->manager, gid);
}

static void _on_row_remove (DownloadRow *row, gpointer ud)
{
    MainWindowPrivate *p = ud;
    download_manager_remove (p->manager, download_row_get_gid (row));
}

static void _on_item_added (DownloadManager *mgr, DownloadItem *item, gpointer ud)
{
    (void)mgr;
    MainWindowPrivate *p = ud;
    GtkWidget *row_widget = download_row_new (item);
    g_signal_connect (row_widget, "action-pause", G_CALLBACK(_on_row_pause), p);
    g_signal_connect (row_widget, "action-remove", G_CALLBACK(_on_row_remove), p);
    
    GtkListBoxRow *row = GTK_LIST_BOX_ROW (gtk_list_box_row_new ());
    gtk_list_box_row_set_child (row, row_widget);
    gtk_list_box_append (p->list, GTK_WIDGET(row));
    g_hash_table_insert (p->rows, g_strdup (item->gid), row_widget);
    gtk_stack_set_visible_child_name (p->stack, "list");
}

static void _on_item_updated (DownloadManager *mgr, DownloadItem *item, gpointer ud)
{
    (void)mgr;
    MainWindowPrivate *p = ud;
    DownloadRow *dr = g_hash_table_lookup (p->rows, item->gid);
    if (dr) download_row_update (dr, item);
    gtk_list_box_invalidate_filter (p->list);
}

static void _on_item_removed (DownloadManager *mgr, const gchar *gid, gpointer ud)
{
    (void)mgr;
    MainWindowPrivate *p = ud;
    DownloadRow *dr = g_hash_table_lookup (p->rows, gid);
    if (dr) {
        GtkWidget *row = gtk_widget_get_parent (GTK_WIDGET(dr));
        if (GTK_IS_LIST_BOX_ROW (row))
            gtk_list_box_remove (p->list, row);
        g_hash_table_remove (p->rows, gid);
    }
    if (g_hash_table_size (p->rows) == 0)
        gtk_stack_set_visible_child_name (p->stack, "empty");
}

static void _on_stats_changed (DownloadManager *mgr, gint64 dl, gint64 ul, gint active, gpointer ud)
{
    (void)mgr;
    MainWindowPrivate *p = ud;
    statusbar_update (p->statbar, dl, ul, active, 0);
}

static void _on_filter_changed (Sidebar *sb, const gchar *filter, gpointer ud)
{
    (void)sb;
    MainWindowPrivate *p = ud;
    g_free (p->active_filter);
    p->active_filter = g_strdup (filter);
    gtk_list_box_invalidate_filter (p->list);
}

static void _on_search_changed (GtkSearchEntry *se, gpointer ud)
{
    (void)se;
    MainWindowPrivate *p = ud;
    gtk_list_box_invalidate_filter (p->list);
}

static gboolean
_on_key_pressed (GtkEventControllerKey *ctrl, guint keyval, guint keycode,
                 GdkModifierType state, gpointer ud)
{
    (void)ctrl; (void)keycode;
    if ((state & GDK_CONTROL_MASK) && keyval == GDK_KEY_n) {
        main_window_show_add_dialog (VXAP_MAIN_WINDOW(ud));
        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;
}

/* ─── Build ─── */
GtkWidget *
main_window_new (GtkApplication *app, DownloadManager *manager)
{
    MainWindow *self = g_object_new (VXAP_TYPE_MAIN_WINDOW,
                                     "application", app,
                                     "title", "VXAP Downloader",
                                     "default-width",  1000,
                                     "default-height", 640,
                                     NULL);
    MainWindowPrivate *p = main_window_get_instance_private (self);
    p->manager = manager;

    gtk_widget_add_css_class (GTK_WIDGET(self), "main-window");

    /* CSS */
    GtkCssProvider *css = gtk_css_provider_new ();
    gtk_css_provider_load_from_resource (css, "/com/vxap/downloader/style.css");
    gtk_style_context_add_provider_for_display (gdk_display_get_default (),
        GTK_STYLE_PROVIDER (css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref (css);

    /* Root layout */
    GtkBox *root = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

    /* Sidebar */
    p->sidebar = VXAP_SIDEBAR (sidebar_new ());
    gtk_box_append (root, GTK_WIDGET(p->sidebar));
    g_signal_connect (p->sidebar, "filter-changed", G_CALLBACK(_on_filter_changed), p);
    gtk_box_append (root, GTK_WIDGET(gtk_separator_new (GTK_ORIENTATION_VERTICAL)));

    /* Right panel */
    GtkBox *right = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
    gtk_widget_set_hexpand (GTK_WIDGET(right), TRUE);

    /* Header bar */
    GtkHeaderBar *hbar = GTK_HEADER_BAR (gtk_header_bar_new ());
    gtk_header_bar_set_show_title_buttons (hbar, TRUE);

    GtkButton *btn_add = GTK_BUTTON (gtk_button_new_from_icon_name ("list-add-symbolic"));
    gtk_widget_set_tooltip_text (GTK_WIDGET(btn_add), "Add Download (Ctrl+N)");
    gtk_widget_add_css_class (GTK_WIDGET(btn_add), "suggested-action");
    gtk_header_bar_pack_start (hbar, GTK_WIDGET(btn_add));

    GtkButton *btn_pause_all  = GTK_BUTTON (gtk_button_new_from_icon_name ("media-playback-pause-symbolic"));
    GtkButton *btn_resume_all = GTK_BUTTON (gtk_button_new_from_icon_name ("media-playback-start-symbolic"));
    gtk_header_bar_pack_start (hbar, GTK_WIDGET(btn_pause_all));
    gtk_header_bar_pack_start (hbar, GTK_WIDGET(btn_resume_all));

    p->search = GTK_SEARCH_ENTRY (gtk_search_entry_new ());
    gtk_search_entry_set_placeholder_text (p->search, "Search...");
    gtk_widget_set_size_request (GTK_WIDGET(p->search), 220, -1);
    gtk_header_bar_set_title_widget (hbar, GTK_WIDGET(p->search));
    gtk_window_set_titlebar (GTK_WINDOW(self), GTK_WIDGET(hbar));

    g_signal_connect_swapped (btn_add,       "clicked", G_CALLBACK(main_window_show_add_dialog), self);
    g_signal_connect_swapped (btn_pause_all, "clicked", G_CALLBACK(download_manager_pause_all),  manager);
    g_signal_connect_swapped (btn_resume_all,"clicked", G_CALLBACK(download_manager_resume_all), manager);
    g_signal_connect (p->search, "search-changed", G_CALLBACK(_on_search_changed), p);

    /* Scrolled download list */
    GtkScrolledWindow *sw = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new ());
    gtk_scrolled_window_set_policy (sw, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand (GTK_WIDGET(sw), TRUE);

    p->list = GTK_LIST_BOX (gtk_list_box_new ());
    gtk_list_box_set_selection_mode (p->list, GTK_SELECTION_SINGLE);
    gtk_list_box_set_filter_func (p->list, _filter_func, p, NULL);
    gtk_widget_add_css_class (GTK_WIDGET(p->list), "download-list");
    gtk_scrolled_window_set_child (sw, GTK_WIDGET(p->list));

    /* Empty state */
    GtkBox *empty = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 16));
    gtk_widget_set_halign (GTK_WIDGET(empty), GTK_ALIGN_CENTER);
    gtk_widget_set_valign (GTK_WIDGET(empty), GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand (GTK_WIDGET(empty), TRUE);
    gtk_widget_set_vexpand (GTK_WIDGET(empty), TRUE);
    GtkImage *empty_ico = GTK_IMAGE (gtk_image_new_from_icon_name ("folder-download-symbolic"));
    gtk_image_set_pixel_size (empty_ico, 64);
    gtk_widget_add_css_class (GTK_WIDGET(empty_ico), "dim-label");
    GtkLabel *empty_lbl = GTK_LABEL (gtk_label_new ("No Downloads\nPress + to add link"));
    gtk_label_set_justify (empty_lbl, GTK_JUSTIFY_CENTER);
    gtk_widget_add_css_class (GTK_WIDGET(empty_lbl), "dim-label");
    gtk_box_append (empty, GTK_WIDGET(empty_ico));
    gtk_box_append (empty, GTK_WIDGET(empty_lbl));

    p->stack = GTK_STACK (gtk_stack_new ());
    gtk_stack_add_named (p->stack, GTK_WIDGET(empty), "empty");
    gtk_stack_add_named (p->stack, GTK_WIDGET(sw),    "list");
    gtk_stack_set_visible_child_name (p->stack, "empty");
    gtk_box_append (right, GTK_WIDGET(p->stack));

    /* Status bar */
    p->statbar = VXAP_STATUSBAR (statusbar_new ());
    gtk_box_append (right, GTK_WIDGET(gtk_separator_new (GTK_ORIENTATION_HORIZONTAL)));
    gtk_box_append (right, GTK_WIDGET(p->statbar));

    gtk_box_append (root, GTK_WIDGET(right));
    gtk_window_set_child (GTK_WINDOW(self), GTK_WIDGET(root));

    /* Connect manager signals */
    g_signal_connect (manager, "item-added",    G_CALLBACK(_on_item_added),    p);
    g_signal_connect (manager, "item-updated",  G_CALLBACK(_on_item_updated),  p);
    g_signal_connect (manager, "item-removed",  G_CALLBACK(_on_item_removed),  p);
    g_signal_connect (manager, "stats-changed", G_CALLBACK(_on_stats_changed), p);

    /* Keyboard shortcuts */
    GtkEventControllerKey *kc = GTK_EVENT_CONTROLLER_KEY (gtk_event_controller_key_new ());
    g_signal_connect (kc, "key-pressed", G_CALLBACK(_on_key_pressed), self);
    gtk_widget_add_controller (GTK_WIDGET(self), GTK_EVENT_CONTROLLER(kc));

    return GTK_WIDGET(self);
}

void
main_window_show_add_dialog (MainWindow *self)
{
    MainWindowPrivate *p = main_window_get_instance_private (self);
    GtkWidget *dlg = add_dialog_new (GTK_WINDOW(self), p->manager, NULL);
    gtk_widget_set_visible (dlg, TRUE);
}
