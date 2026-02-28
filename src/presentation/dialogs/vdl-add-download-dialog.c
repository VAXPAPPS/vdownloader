/*
 * VXAPDownloader - Add Download Dialog
 * Dialog for adding a new download. When "Download" is clicked,
 * creates a VdlDownloadItem and emits "download-requested" signal.
 */

#include "vdl-add-download-dialog.h"
#include "../../core/vdl-types.h"
#include "../../core/vdl-utils.h"
#include "../../domain/entities/vdl-download-item.h"

struct _VdlAddDownloadDialog {
    AdwWindow parent_instance;
    GtkWidget *url_entry;
    GtkWidget *filename_entry;
    GtkWidget *path_label;
    GtkWidget *segments_spin;
    GtkWidget *download_btn;
    char      *save_path;
};

G_DEFINE_TYPE(VdlAddDownloadDialog, vdl_add_download_dialog, ADW_TYPE_WINDOW)

enum {
    SIGNAL_DOWNLOAD_REQUESTED,
    N_SIGNALS
};
static guint dialog_signals[N_SIGNALS] = { 0 };

static void
on_url_changed (GtkEditable *editable, gpointer user_data)
{
    VdlAddDownloadDialog *self = VDL_ADD_DOWNLOAD_DIALOG (user_data);
    const char *url = gtk_editable_get_text (editable);

    /* Auto-detect filename from URL */
    if (url && *url) {
        g_autofree char *filename = vdl_utils_filename_from_url (url);
        gtk_editable_set_text (GTK_EDITABLE (self->filename_entry), filename);
        gtk_widget_set_sensitive (self->download_btn,
            vdl_utils_is_valid_url (url));
    } else {
        gtk_widget_set_sensitive (self->download_btn, FALSE);
    }
}

static void
on_download_clicked (GtkButton *btn, gpointer user_data)
{
    (void)btn;
    VdlAddDownloadDialog *self = VDL_ADD_DOWNLOAD_DIALOG(user_data);

    const char *url = gtk_editable_get_text (GTK_EDITABLE (self->url_entry));
    const char *filename = gtk_editable_get_text (GTK_EDITABLE (self->filename_entry));

    if (!url || !vdl_utils_is_valid_url (url))
        return;

    /* Create download item */
    VdlDownloadItem *item = vdl_download_item_new (url, self->save_path);

    /* Override filename if user changed it */
    if (filename && *filename) {
        vdl_download_item_set_filename (item, filename);
    }

    /* Set segments */
    int segments = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (self->segments_spin));
    vdl_download_item_set_segments (item, segments);

    /* Emit signal with the download item */
    g_signal_emit (self, dialog_signals[SIGNAL_DOWNLOAD_REQUESTED], 0, item);

    g_object_unref (item);
    gtk_window_close (GTK_WINDOW (self));
}

static void
on_cancel_clicked (GtkButton *btn, gpointer user_data)
{
    (void)btn;
    gtk_window_close (GTK_WINDOW (user_data));
}

static void
vdl_add_download_dialog_finalize (GObject *obj)
{
    VdlAddDownloadDialog *self = VDL_ADD_DOWNLOAD_DIALOG (obj);
    g_free (self->save_path);
    G_OBJECT_CLASS (vdl_add_download_dialog_parent_class)->finalize (obj);
}

static void
vdl_add_download_dialog_class_init (VdlAddDownloadDialogClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS (klass);
    obj_class->finalize = vdl_add_download_dialog_finalize;

    /**
     * VdlAddDownloadDialog::download-requested:
     * @dialog: the dialog
     * @item: the VdlDownloadItem to download
     */
    dialog_signals[SIGNAL_DOWNLOAD_REQUESTED] = g_signal_new (
        "download-requested",
        VDL_TYPE_ADD_DOWNLOAD_DIALOG,
        G_SIGNAL_RUN_LAST,
        0, NULL, NULL, NULL,
        G_TYPE_NONE, 1,
        VDL_TYPE_DOWNLOAD_ITEM
    );
}

static void
vdl_add_download_dialog_init (VdlAddDownloadDialog *self)
{
    /* Default save path */
    self->save_path = g_build_filename (
        g_get_user_special_dir (G_USER_DIRECTORY_DOWNLOAD), NULL);

    gtk_window_set_default_size (GTK_WINDOW (self), 520, -1);
    gtk_window_set_modal (GTK_WINDOW (self), TRUE);
    gtk_window_set_decorated (GTK_WINDOW (self), FALSE);
    gtk_widget_add_css_class (GTK_WIDGET (self), "vdl-dialog");

    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_set_margin_start (vbox, 24);
    gtk_widget_set_margin_end (vbox, 24);
    gtk_widget_set_margin_top (vbox, 20);
    gtk_widget_set_margin_bottom (vbox, 20);
    adw_window_set_content (ADW_WINDOW (self), vbox);

    /* Title */
    GtkWidget *title = gtk_label_new ("New Download");
    gtk_widget_add_css_class (title, "vdl-content-title");
    gtk_widget_set_halign (title, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (vbox), title);

    /* URL Input */
    GtkWidget *url_label = gtk_label_new ("Download URL");
    gtk_widget_set_halign (url_label, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (vbox), url_label);

    self->url_entry = gtk_entry_new ();
    g_object_set_data (G_OBJECT (self), "url_entry", self->url_entry);
    gtk_entry_set_placeholder_text (GTK_ENTRY (self->url_entry),
        "https://example.com/file.zip");
    g_signal_connect (self->url_entry, "changed",
                      G_CALLBACK (on_url_changed), self);
    gtk_box_append (GTK_BOX (vbox), self->url_entry);

    /* Filename */
    GtkWidget *name_label = gtk_label_new ("Filename");
    gtk_widget_set_halign (name_label, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (vbox), name_label);

    self->filename_entry = gtk_entry_new ();
    gtk_entry_set_placeholder_text (GTK_ENTRY (self->filename_entry),
        "Auto-detect from URL");
    gtk_box_append (GTK_BOX (vbox), self->filename_entry);

    /* Save path + Segments row */
    GtkWidget *options_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);

    GtkWidget *path_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand (path_box, TRUE);
    GtkWidget *plabel = gtk_label_new ("Save to");
    gtk_widget_set_halign (plabel, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (path_box), plabel);

    self->path_label = gtk_label_new (self->save_path);
    gtk_label_set_ellipsize (GTK_LABEL (self->path_label), PANGO_ELLIPSIZE_START);
    gtk_widget_set_halign (self->path_label, GTK_ALIGN_START);
    gtk_widget_set_opacity (self->path_label, 0.6);
    gtk_box_append (GTK_BOX (path_box), self->path_label);

    gtk_box_append (GTK_BOX (options_row), path_box);

    GtkWidget *seg_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *seg_label = gtk_label_new ("Segments");
    gtk_widget_set_halign (seg_label, GTK_ALIGN_START);
    gtk_box_append (GTK_BOX (seg_box), seg_label);
    self->segments_spin = gtk_spin_button_new_with_range (1, 32, 1);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (self->segments_spin), 8);
    gtk_box_append (GTK_BOX (seg_box), self->segments_spin);
    gtk_box_append (GTK_BOX (options_row), seg_box);

    gtk_box_append (GTK_BOX (vbox), options_row);

    /* Buttons */
    GtkWidget *btn_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign (btn_row, GTK_ALIGN_END);
    gtk_widget_set_margin_top (btn_row, 8);

    GtkWidget *cancel_btn = gtk_button_new_with_label ("Cancel");
    gtk_widget_add_css_class (cancel_btn, "vdl-action-btn");
    g_signal_connect (cancel_btn, "clicked",
                      G_CALLBACK (on_cancel_clicked), self);
    gtk_box_append (GTK_BOX (btn_row), cancel_btn);

    self->download_btn = gtk_button_new_with_label ("Download");
    gtk_widget_add_css_class (self->download_btn, "vdl-btn-primary");
    gtk_widget_set_sensitive (self->download_btn, FALSE);
    g_signal_connect (self->download_btn, "clicked",
                      G_CALLBACK (on_download_clicked), self);
    gtk_box_append (GTK_BOX (btn_row), self->download_btn);

    gtk_box_append (GTK_BOX (vbox), btn_row);
}

VdlAddDownloadDialog *
vdl_add_download_dialog_new (GtkWindow *parent)
{
    VdlAddDownloadDialog *self = g_object_new (VDL_TYPE_ADD_DOWNLOAD_DIALOG, NULL);
    gtk_window_set_transient_for (GTK_WINDOW (self), parent);
    return self;
}
