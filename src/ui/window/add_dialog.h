#pragma once

#include <gtk/gtk.h>
#include "core/download_manager.h"

GtkWidget *add_dialog_new (GtkWindow *parent, DownloadManager *manager, const gchar *preset_url);
