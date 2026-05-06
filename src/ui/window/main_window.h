#pragma once

#include <gtk/gtk.h>
#include "core/download_manager.h"

#define VXAP_TYPE_MAIN_WINDOW (main_window_get_type())
G_DECLARE_FINAL_TYPE(MainWindow, main_window, VXAP, MAIN_WINDOW, GtkApplicationWindow)

GtkWidget *main_window_new     (GtkApplication *app, DownloadManager *manager);
void       main_window_show_add_dialog (MainWindow *self);
