#pragma once

#include <glib.h>

gchar   *utils_format_size         (gint64 bytes);
gchar   *utils_format_speed        (gint64 bps);
gchar   *utils_format_eta          (gint64 remaining, gint64 speed);
gboolean utils_is_valid_url        (const gchar *url);
gchar   *utils_basename_from_url   (const gchar *url);
gchar   *utils_get_host_from_url   (const gchar *url);
gchar   *utils_get_default_download_dir (void);
