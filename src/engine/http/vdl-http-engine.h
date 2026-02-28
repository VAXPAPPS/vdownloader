/*
 * VXAPDownloader - HTTP Download Engine
 * Multi-segment download engine using libcurl and pthreads
 */

#ifndef VDL_HTTP_ENGINE_H
#define VDL_HTTP_ENGINE_H

#include <glib-object.h>
#include "../../domain/entities/vdl-download-item.h"

G_BEGIN_DECLS

#define VDL_TYPE_HTTP_ENGINE (vdl_http_engine_get_type())
G_DECLARE_FINAL_TYPE (VdlHttpEngine, vdl_http_engine, VDL, HTTP_ENGINE, GObject)

VdlHttpEngine *vdl_http_engine_new (void);

/* Start a download (runs in background thread) */
void vdl_http_engine_start_download (VdlHttpEngine *self, VdlDownloadItem *item);

/* Pause a running download */
void vdl_http_engine_pause_download (VdlHttpEngine *self, const char *id);

/* Resume a paused download */
void vdl_http_engine_resume_download (VdlHttpEngine *self, const char *id);

/* Cancel and remove a download */
void vdl_http_engine_cancel_download (VdlHttpEngine *self, const char *id);

G_END_DECLS

#endif /* VDL_HTTP_ENGINE_H */
