#include "vdl-preferences-dialog.h"
struct _VdlPreferencesDialog { AdwWindow parent; };
G_DEFINE_TYPE(VdlPreferencesDialog, vdl_preferences_dialog, ADW_TYPE_WINDOW)
static void vdl_preferences_dialog_class_init(VdlPreferencesDialogClass *k) {}
static void vdl_preferences_dialog_init(VdlPreferencesDialog *s) {
    gtk_window_set_default_size(GTK_WINDOW(s), 600, 500);
    gtk_window_set_title(GTK_WINDOW(s), "Preferences");
    GtkWidget *lbl = gtk_label_new("Preferences - Coming Soon");
    adw_window_set_content(ADW_WINDOW(s), lbl);
}
VdlPreferencesDialog *vdl_preferences_dialog_new(GtkWindow *parent) {
    VdlPreferencesDialog *s = g_object_new(VDL_TYPE_PREFERENCES_DIALOG, NULL);
    gtk_window_set_transient_for(GTK_WINDOW(s), parent);
    return s;
}
