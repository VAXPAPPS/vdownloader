/* VXAPDownloader - SQLite Repository */
#ifndef VDL_SQLITE_REPO_H
#define VDL_SQLITE_REPO_H
#include <glib-object.h>
#include <sqlite3.h>
G_BEGIN_DECLS
#define VDL_TYPE_SQLITE_REPO (vdl_sqlite_repo_get_type())
G_DECLARE_FINAL_TYPE (VdlSqliteRepo, vdl_sqlite_repo, VDL, SQLITE_REPO, GObject)
VdlSqliteRepo *vdl_sqlite_repo_new (const char *db_path);
sqlite3 *vdl_sqlite_repo_get_db (VdlSqliteRepo *self);
gboolean vdl_sqlite_repo_execute (VdlSqliteRepo *self, const char *sql);
G_END_DECLS
#endif
