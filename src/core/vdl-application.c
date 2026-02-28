/*
 * VXAPDownloader - Professional Download Manager for VAXP-OS
 * Copyright (C) 2026 VAXP Organization
 */

#include "vdl-application.h"
#include "vdl-config.h"
#include "vdl-logger.h"
#include "../data/repositories/vdl-sqlite-repo.h"
#include "../data/migrations/vdl-db-migration.h"
#include "../engine/http/vdl-http-engine.h"
#include "../engine/scheduler/vdl-queue-manager.h"
#include "../engine/torrent/vdl-torrent-engine.h"
#include "../presentation/windows/vdl-main-window.h"

struct _VdlApplication {
    AdwApplication   parent_instance;

    VdlSqliteRepo       *repo;
    VdlHttpEngine       *http_engine;
    VdlQueueManager     *queue_manager;
    VdlConfig           *config;
    VdlTorrentEngine    *torrent_engine;
    VdlMainWindow       *main_window;
};

G_DEFINE_TYPE (VdlApplication, vdl_application, ADW_TYPE_APPLICATION)

static void
vdl_application_activate (GApplication *app)
{
    VdlApplication *self = VDL_APPLICATION (app);

    if (self->main_window == NULL) {
        self->main_window = vdl_main_window_new (self);
    }

    gtk_window_present (GTK_WINDOW (self->main_window));
}

static void
vdl_application_startup (GApplication *app)
{
    VdlApplication *self = VDL_APPLICATION (app);

    G_APPLICATION_CLASS (vdl_application_parent_class)->startup (app);

    vdl_logger_init ();
    vdl_logger_info ("VXAPDownloader v%s starting...", VDL_APP_VERSION);

    /* Load CSS theme */
    GtkCssProvider *css_provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_resource (css_provider, "/org/vaxp/downloader/css/style.css");
    gtk_style_context_add_provider_for_display (
        gdk_display_get_default (),
        GTK_STYLE_PROVIDER (css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref (css_provider);

    /* Initialize config */
    self->config = vdl_config_new ();

    /* Initialize database */
    g_autofree char *db_path = g_build_filename (
        g_get_user_data_dir (), "vxap-downloader", "downloads.db", NULL
    );
    g_autofree char *db_dir = g_path_get_dirname (db_path);
    g_mkdir_with_parents (db_dir, 0755);

    self->repo = vdl_sqlite_repo_new (db_path);
    vdl_db_migration_run (self->repo);

    /* Initialize engines */
    self->http_engine = vdl_http_engine_new ();
    self->queue_manager = vdl_queue_manager_new (self->http_engine);

    /* Initialize torrent engine */
    const char *dl_dir = vdl_config_get_download_dir (self->config);
    self->torrent_engine = vdl_torrent_engine_new (dl_dir);

    vdl_logger_info ("Application startup complete.");
}

static void
vdl_application_shutdown (GApplication *app)
{
    VdlApplication *self = VDL_APPLICATION (app);

    vdl_logger_info ("Application shutting down...");

    g_clear_object (&self->queue_manager);
    g_clear_object (&self->http_engine);
    g_clear_object (&self->repo);
    g_clear_object (&self->config);

    if (self->torrent_engine) {
        vdl_torrent_engine_destroy (self->torrent_engine);
        self->torrent_engine = NULL;
    }

    vdl_logger_shutdown ();

    G_APPLICATION_CLASS (vdl_application_parent_class)->shutdown (app);
}

static void
vdl_application_class_init (VdlApplicationClass *klass)
{
    GApplicationClass *app_class = G_APPLICATION_CLASS (klass);
    app_class->activate = vdl_application_activate;
    app_class->startup  = vdl_application_startup;
    app_class->shutdown = vdl_application_shutdown;
}

static void
vdl_application_init (VdlApplication *self)
{
    self->repo            = NULL;
    self->http_engine     = NULL;
    self->queue_manager   = NULL;
    self->config          = NULL;
    self->torrent_engine  = NULL;
    self->main_window     = NULL;
}

VdlApplication *
vdl_application_new (void)
{
    return g_object_new (VDL_TYPE_APPLICATION,
                         "application-id", VDL_APP_ID,
                         "flags", G_APPLICATION_DEFAULT_FLAGS,
                         NULL);
}

VdlSqliteRepo *
vdl_application_get_repo (VdlApplication *self)
{
    g_return_val_if_fail (VDL_IS_APPLICATION (self), NULL);
    return self->repo;
}

VdlHttpEngine *
vdl_application_get_http_engine (VdlApplication *self)
{
    g_return_val_if_fail (VDL_IS_APPLICATION (self), NULL);
    return self->http_engine;
}

VdlQueueManager *
vdl_application_get_queue_manager (VdlApplication *self)
{
    g_return_val_if_fail (VDL_IS_APPLICATION (self), NULL);
    return self->queue_manager;
}

VdlConfig *
vdl_application_get_config (VdlApplication *self)
{
    g_return_val_if_fail (VDL_IS_APPLICATION (self), NULL);
    return self->config;
}

void *
vdl_application_get_torrent_engine (VdlApplication *self)
{
    g_return_val_if_fail (VDL_IS_APPLICATION (self), NULL);
    return self->torrent_engine;
}
