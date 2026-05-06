#pragma once

#include <glib.h>

/* Application-wide configuration, backed by GKeyFile */

typedef struct {
    gchar   *download_dir;
    gint     max_concurrent;       /* max simultaneous downloads */
    gint     max_connections;      /* connections per server */
    gchar   *aria2_host;
    gint     aria2_port;
    gchar   *aria2_secret;
    gboolean start_minimized;
    gboolean dark_mode;
} VxapConfig;

VxapConfig *vxap_config_load  (void);
void        vxap_config_save  (const VxapConfig *cfg);
void        vxap_config_free  (VxapConfig *cfg);
