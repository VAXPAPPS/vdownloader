/*
 * VXAPDownloader - Database Migration
 * Creates and upgrades the SQLite schema
 */
#include "vdl-db-migration.h"
#include "../../core/vdl-logger.h"

static const char *MIGRATION_V1 =
    "CREATE TABLE IF NOT EXISTS downloads ("
    "  id TEXT PRIMARY KEY,"
    "  url TEXT NOT NULL,"
    "  filename TEXT NOT NULL,"
    "  save_path TEXT NOT NULL,"
    "  total_size INTEGER DEFAULT 0,"
    "  downloaded INTEGER DEFAULT 0,"
    "  status INTEGER DEFAULT 0,"
    "  type INTEGER DEFAULT 0,"
    "  priority INTEGER DEFAULT 1,"
    "  segments INTEGER DEFAULT 8,"
    "  category_id TEXT,"
    "  checksum TEXT,"
    "  error_message TEXT,"
    "  created_at INTEGER,"
    "  completed_at INTEGER,"
    "  FOREIGN KEY (category_id) REFERENCES categories(id)"
    ");"
    "CREATE TABLE IF NOT EXISTS categories ("
    "  id TEXT PRIMARY KEY,"
    "  name TEXT NOT NULL,"
    "  icon_name TEXT,"
    "  color TEXT,"
    "  save_path TEXT,"
    "  type INTEGER DEFAULT 0"
    ");"
    "CREATE TABLE IF NOT EXISTS segments ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  download_id TEXT NOT NULL,"
    "  segment_index INTEGER NOT NULL,"
    "  start_byte INTEGER NOT NULL,"
    "  end_byte INTEGER NOT NULL,"
    "  downloaded INTEGER DEFAULT 0,"
    "  status INTEGER DEFAULT 0,"
    "  FOREIGN KEY (download_id) REFERENCES downloads(id) ON DELETE CASCADE"
    ");"
    "CREATE TABLE IF NOT EXISTS statistics ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  total_downloaded INTEGER DEFAULT 0,"
    "  total_uploaded INTEGER DEFAULT 0,"
    "  files_completed INTEGER DEFAULT 0,"
    "  date TEXT NOT NULL"
    ");"
    "CREATE TABLE IF NOT EXISTS schema_version ("
    "  version INTEGER NOT NULL"
    ");"
    "INSERT OR IGNORE INTO schema_version (version) VALUES (1);";

void vdl_db_migration_run (VdlSqliteRepo *repo)
{
    if (!repo) return;

    vdl_logger_info ("Running database migrations...");
    gboolean ok = vdl_sqlite_repo_execute (repo, MIGRATION_V1);
    if (ok)
        vdl_logger_info ("Database schema ready (v1).");
    else
        vdl_logger_error ("Database migration failed!");
}
