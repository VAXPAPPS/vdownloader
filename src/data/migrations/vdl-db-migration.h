/* VXAPDownloader - Database Migration */
#ifndef VDL_DB_MIGRATION_H
#define VDL_DB_MIGRATION_H
#include "../repositories/vdl-sqlite-repo.h"
void vdl_db_migration_run (VdlSqliteRepo *repo);
#endif
