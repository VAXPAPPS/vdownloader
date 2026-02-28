/* VXAPDownloader - Chunk Manager (stub) */
#ifndef VDL_CHUNK_MANAGER_H
#define VDL_CHUNK_MANAGER_H
#include <glib-object.h>
G_BEGIN_DECLS
#define VDL_TYPE_CHUNK_MANAGER (vdl_chunk_manager_get_type())
G_DECLARE_FINAL_TYPE(VdlChunkManager, vdl_chunk_manager, VDL, CHUNK_MANAGER, GObject)
VdlChunkManager *vdl_chunk_manager_new (void);
G_END_DECLS
#endif
