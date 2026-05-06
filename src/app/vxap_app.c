#include "app/vxap_app.h"
#include "app/vxap_config.h"
#include "aria2/aria2_client.h"
#include "core/download_manager.h"
#include "ui/window/main_window.h"
#include <gtk/gtk.h>

#ifdef HAVE_ADWAITA
#include <adwaita.h>
#endif

struct _VxapApp {
#ifdef HAVE_ADWAITA
    AdwApplication parent;
#else
    GtkApplication parent;
#endif
};

typedef struct {
    VxapConfig      *config;
    Aria2Client     *aria2;
    DownloadManager *manager;
} VxapAppPrivate;

#ifdef HAVE_ADWAITA
G_DEFINE_TYPE_WITH_PRIVATE (VxapApp, vxap_app, ADW_TYPE_APPLICATION)
#else
G_DEFINE_TYPE_WITH_PRIVATE (VxapApp, vxap_app, GTK_TYPE_APPLICATION)
#endif

static void
vxap_app_finalize (GObject *obj)
{
    VxapApp *self = VXAP_APP (obj);
    VxapAppPrivate *p = vxap_app_get_instance_private (self);
    if (p->manager) { download_manager_stop (p->manager); g_object_unref (p->manager); }
    if (p->aria2)   { aria2_client_free (p->aria2); }
    if (p->config)  { vxap_config_free (p->config); }
    G_OBJECT_CLASS(vxap_app_parent_class)->finalize (obj);
}

static void
vxap_app_activate (GApplication *app)
{
    VxapApp *self = VXAP_APP (app);
    VxapAppPrivate *p = vxap_app_get_instance_private (self);

    /* Load config */
    p->config = vxap_config_load ();

#ifdef HAVE_ADWAITA
    AdwStyleManager *sm = adw_style_manager_get_default ();
    adw_style_manager_set_color_scheme (sm,
        p->config->dark_mode ? ADW_COLOR_SCHEME_FORCE_DARK : ADW_COLOR_SCHEME_DEFAULT);
#endif

    /* Connect to aria2 */
    p->aria2 = aria2_client_new (p->config->aria2_host,
                                  p->config->aria2_port,
                                  p->config->aria2_secret);

    if (!aria2_client_test_connection (p->aria2)) {
        g_warning ("Cannot connect to aria2 RPC at %s:%d — trying to launch aria2c...",
                   p->config->aria2_host, p->config->aria2_port);
        /* Auto-launch aria2c in daemon mode */
        gchar *cmd = g_strdup_printf (
            "aria2c --enable-rpc --rpc-listen-all=false "
            "--rpc-listen-port=%d --daemon=true "
            "--continue=true --max-concurrent-downloads=%d "
            "--user-agent=\"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 Chrome/120.0.0.0 Safari/537.36\"",
            p->config->aria2_port, p->config->max_concurrent);
        g_spawn_command_line_async (cmd, NULL);
        g_free (cmd);
        g_usleep (800000); /* wait 0.8s for aria2c to start */
        aria2_client_test_connection (p->aria2);
    }

    /* Create download manager and start monitor */
    p->manager = download_manager_new (p->aria2);
    download_manager_start (p->manager);

    /* Create and show main window */
    GtkWidget *win = main_window_new (GTK_APPLICATION(app), p->manager);
    gtk_window_present (GTK_WINDOW(win));
}

static void vxap_app_class_init (VxapAppClass *klass)
{
    GObjectClass *oc = G_OBJECT_CLASS (klass);
    oc->finalize = vxap_app_finalize;
    G_APPLICATION_CLASS(klass)->activate = vxap_app_activate;
}

static void vxap_app_init (VxapApp *self) {}

VxapApp *
vxap_app_new (void)
{
    return g_object_new (VXAP_TYPE_APPLICATION,
                         "application-id", APP_ID,
                         "flags", G_APPLICATION_DEFAULT_FLAGS,
                         NULL);
}
