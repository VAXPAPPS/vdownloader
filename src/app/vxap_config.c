#include "app/vxap_config.h"
#include "aria2/aria2_client.h"
#include <glib.h>
#include <glib/gstdio.h>

#define CFG_GROUP  "VXAPDownloader"
#define CFG_FILE   "config.ini"

static gchar *
_config_path (void)
{
    return g_build_filename (g_get_user_config_dir (), "vxap-downloader", CFG_FILE, NULL);
}

static gchar *
_default_dl_dir (void)
{
    const gchar *d = g_get_user_special_dir (G_USER_DIRECTORY_DOWNLOAD);
    return g_strdup (d ? d : g_get_home_dir ());
}

VxapConfig *
vxap_config_load (void)
{
    VxapConfig *cfg = g_new0 (VxapConfig, 1);
    /* Defaults */
    cfg->download_dir    = _default_dl_dir ();
    cfg->max_concurrent  = 3;
    cfg->max_connections = 8;
    cfg->aria2_host      = g_strdup (ARIA2_DEFAULT_HOST);
    cfg->aria2_port      = ARIA2_DEFAULT_PORT;
    cfg->aria2_secret    = g_strdup ("");
    cfg->dark_mode       = FALSE;

    gchar *path = _config_path ();
    GKeyFile *kf = g_key_file_new ();
    if (g_key_file_load_from_file (kf, path, G_KEY_FILE_NONE, NULL)) {
        gchar *d = g_key_file_get_string  (kf, CFG_GROUP, "download-dir",    NULL);
        if (d) { g_free (cfg->download_dir); cfg->download_dir = d; }
        gint ci = g_key_file_get_integer (kf, CFG_GROUP, "max-concurrent",  NULL);
        if (ci) cfg->max_concurrent = ci;
        gint co = g_key_file_get_integer (kf, CFG_GROUP, "max-connections", NULL);
        if (co) cfg->max_connections = co;
        gchar *h = g_key_file_get_string (kf, CFG_GROUP, "aria2-host",   NULL);
        if (h) { g_free (cfg->aria2_host);   cfg->aria2_host = h; }
        gint p  = g_key_file_get_integer (kf, CFG_GROUP, "aria2-port",   NULL);
        if (p)  cfg->aria2_port = p;
        gchar *s = g_key_file_get_string (kf, CFG_GROUP, "aria2-secret", NULL);
        if (s) { g_free (cfg->aria2_secret); cfg->aria2_secret = s; }
        cfg->dark_mode = g_key_file_get_boolean (kf, CFG_GROUP, "dark-mode", NULL);
    }
    g_key_file_free (kf);
    g_free (path);
    return cfg;
}

void
vxap_config_save (const VxapConfig *cfg)
{
    gchar *dir  = g_build_filename (g_get_user_config_dir (), "vxap-downloader", NULL);
    g_mkdir_with_parents (dir, 0755);
    gchar *path = g_build_filename (dir, CFG_FILE, NULL);
    g_free (dir);

    GKeyFile *kf = g_key_file_new ();
    g_key_file_set_string  (kf, CFG_GROUP, "download-dir",    cfg->download_dir);
    g_key_file_set_integer (kf, CFG_GROUP, "max-concurrent",  cfg->max_concurrent);
    g_key_file_set_integer (kf, CFG_GROUP, "max-connections", cfg->max_connections);
    g_key_file_set_string  (kf, CFG_GROUP, "aria2-host",      cfg->aria2_host);
    g_key_file_set_integer (kf, CFG_GROUP, "aria2-port",      cfg->aria2_port);
    g_key_file_set_string  (kf, CFG_GROUP, "aria2-secret",    cfg->aria2_secret);
    g_key_file_set_boolean (kf, CFG_GROUP, "dark-mode",       cfg->dark_mode);
    g_key_file_save_to_file (kf, path, NULL);
    g_key_file_free (kf);
    g_free (path);
}

void
vxap_config_free (VxapConfig *cfg)
{
    if (!cfg) return;
    g_free (cfg->download_dir);
    g_free (cfg->aria2_host);
    g_free (cfg->aria2_secret);
    g_free (cfg);
}
