/*
 * VXAPDownloader - Custom Header Bar (macOS-style title bar)
 * Matches the VAXP-OS ecosystem visual design with colored window
 * control dots and custom title.
 */

#include "vdl-headerbar.h"
#include "../../core/vdl-types.h"

struct _VdlHeaderBar {
    GtkBox       parent_instance;

    GtkWindow   *window;
    GtkWidget   *title_label;
    GtkWidget   *end_box;

    /* Drag state */
    double       drag_start_x;
    double       drag_start_y;
};

G_DEFINE_TYPE (VdlHeaderBar, vdl_headerbar, GTK_TYPE_BOX)

static void
on_close_clicked (GtkButton *btn, gpointer user_data)
{
    VdlHeaderBar *self = VDL_HEADERBAR (user_data);
    gtk_window_close (self->window);
}

static void
on_minimize_clicked (GtkButton *btn, gpointer user_data)
{
    VdlHeaderBar *self = VDL_HEADERBAR (user_data);
    gtk_window_minimize (self->window);
}

static void
on_maximize_clicked (GtkButton *btn, gpointer user_data)
{
    VdlHeaderBar *self = VDL_HEADERBAR (user_data);
    if (gtk_window_is_maximized (self->window))
        gtk_window_unmaximize (self->window);
    else
        gtk_window_maximize (self->window);
}

static void
on_drag_begin (GtkGestureDrag *gesture, double x, double y, gpointer user_data)
{
    VdlHeaderBar *self = VDL_HEADERBAR (user_data);
    self->drag_start_x = x;
    self->drag_start_y = y;
}

static void
on_drag_update (GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data)
{
    VdlHeaderBar *self = VDL_HEADERBAR (user_data);

    GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (self->window));
    if (surface) {
        /* Initiate move using the surface's begin_move */
        gtk_window_present (self->window);
    }
}

static GtkWidget *
create_window_button (const char *css_class, GCallback callback, gpointer user_data)
{
    GtkWidget *btn = gtk_button_new ();
    gtk_widget_add_css_class (btn, "vdl-window-btn");
    gtk_widget_add_css_class (btn, css_class);
    gtk_widget_set_focusable (btn, FALSE);
    gtk_widget_set_tooltip_text (btn,
        g_str_equal(css_class, "vdl-btn-close") ? "Close" :
        g_str_equal(css_class, "vdl-btn-minimize") ? "Minimize" : "Maximize");
    g_signal_connect (btn, "clicked", callback, user_data);
    return btn;
}

static void
vdl_headerbar_class_init (VdlHeaderBarClass *klass)
{
    /* No special class init needed */
}

static void
vdl_headerbar_init (VdlHeaderBar *self)
{
    gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_add_css_class (GTK_WIDGET (self), "vdl-headerbar");
}

VdlHeaderBar *
vdl_headerbar_new (GtkWindow *window)
{
    VdlHeaderBar *self = g_object_new (VDL_TYPE_HEADERBAR, NULL);
    self->window = window;

    /* Left side: Window control buttons */
    GtkWidget *btn_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_end (btn_box, 16);

    GtkWidget *close_btn = create_window_button ("vdl-btn-close",
        G_CALLBACK (on_close_clicked), self);
    GtkWidget *minimize_btn = create_window_button ("vdl-btn-minimize",
        G_CALLBACK (on_minimize_clicked), self);
    GtkWidget *maximize_btn = create_window_button ("vdl-btn-maximize",
        G_CALLBACK (on_maximize_clicked), self);

    gtk_box_append (GTK_BOX (btn_box), close_btn);
    gtk_box_append (GTK_BOX (btn_box), minimize_btn);
    gtk_box_append (GTK_BOX (btn_box), maximize_btn);
    gtk_widget_set_valign (btn_box, GTK_ALIGN_CENTER);

    /* Center: Title */
    self->title_label = gtk_label_new (VDL_APP_NAME);
    gtk_widget_add_css_class (self->title_label, "title");
    gtk_widget_set_hexpand (self->title_label, TRUE);
    gtk_widget_set_halign (self->title_label, GTK_ALIGN_START);

    /* Right side: Extra buttons area */
    self->end_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_valign (self->end_box, GTK_ALIGN_CENTER);

    /* Assemble */
    gtk_box_append (GTK_BOX (self), btn_box);
    gtk_box_append (GTK_BOX (self), self->title_label);
    gtk_box_append (GTK_BOX (self), self->end_box);

    /* Drag gesture for window move */
    GtkGesture *drag = gtk_gesture_drag_new ();
    g_signal_connect (drag, "drag-begin", G_CALLBACK (on_drag_begin), self);
    g_signal_connect (drag, "drag-update", G_CALLBACK (on_drag_update), self);
    gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (drag));

    return self;
}

void
vdl_headerbar_set_title (VdlHeaderBar *self, const char *title)
{
    g_return_if_fail (VDL_IS_HEADERBAR (self));
    gtk_label_set_text (GTK_LABEL (self->title_label), title);
}

void
vdl_headerbar_set_subtitle (VdlHeaderBar *self, const char *subtitle)
{
    /* Could extend to show subtitle if needed */
    (void)self;
    (void)subtitle;
}

GtkWidget *
vdl_headerbar_get_end_box (VdlHeaderBar *self)
{
    g_return_val_if_fail (VDL_IS_HEADERBAR (self), NULL);
    return self->end_box;
}
