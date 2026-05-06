#include "ui/window/add_dialog.h"
#include "core/utils.h"
#include <gtk/gtk.h>

typedef struct {
    DownloadManager *manager;
    GtkEntry        *entry_url;
    GtkEntry        *entry_dir;
    GtkSpinButton   *spin_conns;
    GtkEntry        *entry_out_name;
    GtkEntry        *entry_referer;
} AddDialogData;

static void
_on_add_clicked (GtkButton *btn, gpointer user_data)
{
    GtkWindow *win = GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET(btn)));
    AddDialogData *d = g_object_get_data (G_OBJECT(win), "add-dialog-data");
    const gchar *url = gtk_editable_get_text (GTK_EDITABLE(d->entry_url));
    const gchar *dir = gtk_editable_get_text (GTK_EDITABLE(d->entry_dir));
    gint conns = (gint)gtk_spin_button_get_value (d->spin_conns);
    const gchar *out = gtk_editable_get_text (GTK_EDITABLE(d->entry_out_name));
    const gchar *ref = gtk_editable_get_text (GTK_EDITABLE(d->entry_referer));

    gchar *auto_out = NULL;
    gchar *auto_ref = NULL;
    
    if (!out || !*out) {
        auto_out = utils_basename_from_url (url);
        out = auto_out;
    }
    if (!ref || !*ref) {
        auto_ref = utils_get_host_from_url (url);
        ref = auto_ref;
    }

    if (url && *url)
        download_manager_add_url (d->manager, url, dir, out, ref, conns);

    g_free (auto_out);
    g_free (auto_ref);
    gtk_window_destroy (win);
}

static void
_on_cancel_clicked (GtkButton *btn, gpointer user_data)
{
    GtkWindow *win = GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET(btn)));
    gtk_window_destroy (win);
}

GtkWidget *
add_dialog_new (GtkWindow *parent, DownloadManager *manager, const gchar *preset_url)
{
    GtkWindow *win = GTK_WINDOW (gtk_window_new ());
    gtk_window_set_title (win, "New Download");
    gtk_window_set_modal (win, TRUE);
    gtk_window_set_transient_for (win, parent);
    gtk_window_set_default_size (win, 500, -1);
    gtk_window_set_resizable (win, FALSE);

    AddDialogData *d = g_new0 (AddDialogData, 1);
    d->manager = manager;
    g_object_set_data_full (G_OBJECT(win), "add-dialog-data", d, g_free);

    /* Header bar */
    GtkHeaderBar *hbar = GTK_HEADER_BAR (gtk_header_bar_new ());
    gtk_header_bar_set_show_title_buttons (hbar, FALSE);

    GtkButton *btn_cancel = GTK_BUTTON (gtk_button_new_with_label ("Cancel"));
    GtkButton *btn_add    = GTK_BUTTON (gtk_button_new_with_label ("Download"));
    gtk_widget_add_css_class (GTK_WIDGET(btn_add), "suggested-action");
    gtk_header_bar_pack_start (hbar, GTK_WIDGET(btn_cancel));
    gtk_header_bar_pack_end   (hbar, GTK_WIDGET(btn_add));
    gtk_window_set_titlebar (win, GTK_WIDGET(hbar));

    g_signal_connect (btn_cancel, "clicked", G_CALLBACK(_on_cancel_clicked), NULL);
    g_signal_connect (btn_add,    "clicked", G_CALLBACK(_on_add_clicked),    NULL);

    /* Content */
    GtkBox *content = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 12));
    gtk_widget_set_margin_start  (GTK_WIDGET(content), 24);
    gtk_widget_set_margin_end    (GTK_WIDGET(content), 24);
    gtk_widget_set_margin_top    (GTK_WIDGET(content), 16);
    gtk_widget_set_margin_bottom (GTK_WIDGET(content), 20);

    /* URL */
    GtkLabel *lbl_url = GTK_LABEL (gtk_label_new ("URL:"));
    gtk_label_set_xalign (lbl_url, 0);
    gtk_box_append (content, GTK_WIDGET(lbl_url));
    d->entry_url = GTK_ENTRY (gtk_entry_new ());
    gtk_entry_set_placeholder_text (d->entry_url, "https://example.com/file.zip");
    if (preset_url && *preset_url)
        gtk_editable_set_text (GTK_EDITABLE(d->entry_url), preset_url);
    gtk_widget_add_css_class (GTK_WIDGET(d->entry_url), "url-entry");
    gtk_box_append (content, GTK_WIDGET(d->entry_url));

    /* Directory */
    GtkLabel *lbl_dir = GTK_LABEL (gtk_label_new ("Save To:"));
    gtk_label_set_xalign (lbl_dir, 0);
    gtk_box_append (content, GTK_WIDGET(lbl_dir));
    GtkBox *dir_row = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8));
    d->entry_dir = GTK_ENTRY (gtk_entry_new ());
    gchar *default_dir = utils_get_default_download_dir ();
    gtk_editable_set_text (GTK_EDITABLE(d->entry_dir), default_dir);
    g_free (default_dir);
    gtk_widget_set_hexpand (GTK_WIDGET(d->entry_dir), TRUE);
    gtk_box_append (dir_row, GTK_WIDGET(d->entry_dir));
    gtk_box_append (content, GTK_WIDGET(dir_row));

    /* Connections */
    GtkBox *adv = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12));
    GtkLabel *lbl_c = GTK_LABEL (gtk_label_new ("Connections:"));
    gtk_widget_set_hexpand (GTK_WIDGET(lbl_c), TRUE);
    gtk_label_set_xalign (lbl_c, 0);
    d->spin_conns = GTK_SPIN_BUTTON (gtk_spin_button_new_with_range (1, 16, 1));
    gtk_spin_button_set_value (d->spin_conns, 8);
    gtk_box_append (adv, GTK_WIDGET(lbl_c));
    gtk_box_append (adv, GTK_WIDGET(d->spin_conns));
    gtk_box_append (content, GTK_WIDGET(adv));

    /* Advanced Options */
    GtkExpander *exp = GTK_EXPANDER (gtk_expander_new ("Advanced Options"));
    GtkBox *exp_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 12));
    gtk_widget_set_margin_top (GTK_WIDGET(exp_box), 12);

    /* Out name */
    GtkBox *out_row = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12));
    GtkLabel *lbl_out = GTK_LABEL (gtk_label_new ("File Name:"));
    gtk_label_set_xalign (lbl_out, 0);
    d->entry_out_name = GTK_ENTRY (gtk_entry_new ());
    gtk_entry_set_placeholder_text (d->entry_out_name, "Auto-detected if left blank");
    gtk_widget_set_hexpand (GTK_WIDGET(d->entry_out_name), TRUE);
    gtk_box_append (out_row, GTK_WIDGET(lbl_out));
    gtk_box_append (out_row, GTK_WIDGET(d->entry_out_name));
    gtk_box_append (exp_box, GTK_WIDGET(out_row));

    /* Referer */
    GtkBox *ref_row = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12));
    GtkLabel *lbl_ref = GTK_LABEL (gtk_label_new ("Referer:"));
    gtk_label_set_xalign (lbl_ref, 0);
    d->entry_referer = GTK_ENTRY (gtk_entry_new ());
    gtk_entry_set_placeholder_text (d->entry_referer, "Source URL (Referer)");
    gtk_widget_set_hexpand (GTK_WIDGET(d->entry_referer), TRUE);
    gtk_box_append (ref_row, GTK_WIDGET(lbl_ref));
    gtk_box_append (ref_row, GTK_WIDGET(d->entry_referer));
    gtk_box_append (exp_box, GTK_WIDGET(ref_row));

    gtk_expander_set_child (exp, GTK_WIDGET(exp_box));
    gtk_box_append (content, GTK_WIDGET(exp));

    gtk_window_set_child (win, GTK_WIDGET(content));
    return GTK_WIDGET(win);
}
