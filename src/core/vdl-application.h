/*
 * VXAPDownloader - Professional Download Manager for VAXP-OS
 * Copyright (C) 2026 VAXP Organization
 */

#ifndef VDL_APPLICATION_H
#define VDL_APPLICATION_H

#include <adwaita.h>
#include "vdl-types.h"
#include "vdl-enums.h"

G_BEGIN_DECLS

#define VDL_TYPE_APPLICATION (vdl_application_get_type())
G_DECLARE_FINAL_TYPE (VdlApplication, vdl_application, VDL, APPLICATION, AdwApplication)

VdlApplication  *vdl_application_new      (void);
VdlSqliteRepo   *vdl_application_get_repo (VdlApplication *self);
VdlHttpEngine   *vdl_application_get_http_engine (VdlApplication *self);
VdlQueueManager *vdl_application_get_queue_manager (VdlApplication *self);
VdlConfig       *vdl_application_get_config (VdlApplication *self);

/* Returns opaque VdlTorrentEngine* (C wrapper around libtorrent) */
void            *vdl_application_get_torrent_engine (VdlApplication *self);

G_END_DECLS

#endif /* VDL_APPLICATION_H */
