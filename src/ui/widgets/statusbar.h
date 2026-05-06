#pragma once

#include <gtk/gtk.h>

#define VXAP_TYPE_STATUSBAR (statusbar_get_type())
G_DECLARE_FINAL_TYPE(Statusbar, statusbar, VXAP, STATUSBAR, GtkBox)

GtkWidget *statusbar_new   (void);
void statusbar_update (Statusbar *self, gint64 dl_speed, gint64 ul_speed, gint active, gint waiting);
