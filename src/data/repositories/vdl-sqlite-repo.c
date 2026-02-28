/*
 * VXAPDownloader - SQLite Repository
 */
#include "vdl-sqlite-repo.h"
#include "../../core/vdl-logger.h"

struct _VdlSqliteRepo {
    GObject parent_instance;
    sqlite3 *db;
    char *db_path;
};

G_DEFINE_TYPE (VdlSqliteRepo, vdl_sqlite_repo, G_TYPE_OBJECT)

static void vdl_sqlite_repo_finalize (GObject *obj) {
    VdlSqliteRepo *self = VDL_SQLITE_REPO (obj);
    if (self->db) sqlite3_close (self->db);
    g_free (self->db_path);
    G_OBJECT_CLASS (vdl_sqlite_repo_parent_class)->finalize (obj);
}

static void vdl_sqlite_repo_class_init (VdlSqliteRepoClass *klass) {
    G_OBJECT_CLASS (klass)->finalize = vdl_sqlite_repo_finalize;
}

static void vdl_sqlite_repo_init (VdlSqliteRepo *self) {
    self->db = NULL;
}

VdlSqliteRepo *vdl_sqlite_repo_new (const char *db_path) {
    VdlSqliteRepo *self = g_object_new (VDL_TYPE_SQLITE_REPO, NULL);
    self->db_path = g_strdup (db_path);

    int rc = sqlite3_open (db_path, &self->db);
    if (rc != SQLITE_OK) {
        vdl_logger_error ("Failed to open database: %s", sqlite3_errmsg (self->db));
        self->db = NULL;
    } else {
        sqlite3_exec (self->db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
        sqlite3_exec (self->db, "PRAGMA synchronous=NORMAL;", NULL, NULL, NULL);
        sqlite3_exec (self->db, "PRAGMA foreign_keys=ON;", NULL, NULL, NULL);
        vdl_logger_info ("Database opened: %s", db_path);
    }
    return self;
}

sqlite3 *vdl_sqlite_repo_get_db (VdlSqliteRepo *self) { return self->db; }

gboolean vdl_sqlite_repo_execute (VdlSqliteRepo *self, const char *sql) {
    g_return_val_if_fail (VDL_IS_SQLITE_REPO (self) && self->db, FALSE);
    char *err = NULL;
    int rc = sqlite3_exec (self->db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        vdl_logger_error ("SQL error: %s", err);
        sqlite3_free (err);
        return FALSE;
    }
    return TRUE;
}
