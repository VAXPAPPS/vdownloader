#include "ui/widgets/statusbar.h"
#include "core/utils.h"
#include <gtk/gtk.h>

struct _Statusbar {
    GtkBox parent;
    GtkLabel *lbl_dl;
    GtkLabel *lbl_ul;
    GtkLabel *lbl_active;
    GtkLabel *lbl_waiting;
};

G_DEFINE_FINAL_TYPE (Statusbar, statusbar, GTK_TYPE_BOX)

static void statusbar_class_init (StatusbarClass *k) { (void)k; }
static void statusbar_init (Statusbar *self) { (void)self; }

GtkWidget *
statusbar_new (void)
{
    Statusbar *self = g_object_new (VXAP_TYPE_STATUSBAR,
                                    "orientation", GTK_ORIENTATION_HORIZONTAL,
                                    "spacing", 24,
                                    "margin-start", 16,
                                    "margin-end", 16,
                                    "margin-top", 6,
                                    "margin-bottom", 6,
                                    NULL);
    gtk_widget_add_css_class (GTK_WIDGET(self), "statusbar");

    /* Download speed */
    GtkBox *dl_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6));
    GtkImage *dl_ico = GTK_IMAGE (gtk_image_new_from_icon_name ("network-receive-symbolic"));
    self->lbl_dl = GTK_LABEL (gtk_label_new ("0 KB/s"));
    gtk_box_append (dl_box, GTK_WIDGET(dl_ico));
    gtk_box_append (dl_box, GTK_WIDGET(self->lbl_dl));
    gtk_box_append (GTK_BOX(self), GTK_WIDGET(dl_box));

    /* Upload speed */
    GtkBox *ul_box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6));
    GtkImage *ul_ico = GTK_IMAGE (gtk_image_new_from_icon_name ("network-transmit-symbolic"));
    self->lbl_ul = GTK_LABEL (gtk_label_new ("0 KB/s"));
    gtk_box_append (ul_box, GTK_WIDGET(ul_ico));
    gtk_box_append (ul_box, GTK_WIDGET(self->lbl_ul));
    gtk_box_append (GTK_BOX(self), GTK_WIDGET(ul_box));

    gtk_box_append (GTK_BOX(self),
        GTK_WIDGET(gtk_separator_new (GTK_ORIENTATION_VERTICAL)));

    self->lbl_active  = GTK_LABEL (gtk_label_new ("0 Active"));
    self->lbl_waiting = GTK_LABEL (gtk_label_new ("0 Waiting"));
    gtk_widget_add_css_class (GTK_WIDGET(self->lbl_active),  "statusbar-count");
    gtk_widget_add_css_class (GTK_WIDGET(self->lbl_waiting), "statusbar-count");
    gtk_box_append (GTK_BOX(self), GTK_WIDGET(self->lbl_active));
    gtk_box_append (GTK_BOX(self), GTK_WIDGET(self->lbl_waiting));

    return GTK_WIDGET(self);
}

void
statusbar_update (Statusbar *self, gint64 dl_speed, gint64 ul_speed, gint active, gint waiting)
{
    gchar *dl = utils_format_speed (dl_speed);
    gchar *ul = utils_format_speed (ul_speed);
    gtk_label_set_text (self->lbl_dl, dl);
    gtk_label_set_text (self->lbl_ul, ul);
    g_free (dl); g_free (ul);

    gchar *as = g_strdup_printf ("%d Active",    active);
    gchar *ws = g_strdup_printf ("%d Waiting", waiting);
    gtk_label_set_text (self->lbl_active,  as);
    gtk_label_set_text (self->lbl_waiting, ws);
    g_free (as); g_free (ws);
}
