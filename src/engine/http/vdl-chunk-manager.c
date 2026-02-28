#include "vdl-chunk-manager.h"
struct _VdlChunkManager { GObject parent; };
G_DEFINE_TYPE(VdlChunkManager, vdl_chunk_manager, G_TYPE_OBJECT)
static void vdl_chunk_manager_class_init(VdlChunkManagerClass *k) {}
static void vdl_chunk_manager_init(VdlChunkManager *s) {}
VdlChunkManager *vdl_chunk_manager_new(void) { return g_object_new(VDL_TYPE_CHUNK_MANAGER, NULL); }
