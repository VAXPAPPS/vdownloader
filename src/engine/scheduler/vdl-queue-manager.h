/* VXAPDownloader - Queue Manager */
#ifndef VDL_QUEUE_MANAGER_H
#define VDL_QUEUE_MANAGER_H
#include <glib-object.h>
#include "../http/vdl-http-engine.h"
G_BEGIN_DECLS
#define VDL_TYPE_QUEUE_MANAGER (vdl_queue_manager_get_type())
G_DECLARE_FINAL_TYPE(VdlQueueManager, vdl_queue_manager, VDL, QUEUE_MANAGER, GObject)
VdlQueueManager *vdl_queue_manager_new(VdlHttpEngine *engine);
void vdl_queue_manager_add(VdlQueueManager *self, VdlDownloadItem *item);
G_END_DECLS
#endif
