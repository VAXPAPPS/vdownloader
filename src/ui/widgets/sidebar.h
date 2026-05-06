#pragma once

#include <gtk/gtk.h>

#define VXAP_TYPE_SIDEBAR (sidebar_get_type())
G_DECLARE_FINAL_TYPE(Sidebar, sidebar, VXAP, SIDEBAR, GtkBox)

/* Signals: "filter-changed" -> const gchar *filter_id ("all", "active", "paused", "complete", "error") */
GtkWidget  *sidebar_new                (void);
const gchar *sidebar_get_active_filter (Sidebar *self);
