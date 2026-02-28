#include "vdl-queue-manager.h"
#include "../../core/vdl-logger.h"
struct _VdlQueueManager { GObject parent; VdlHttpEngine *engine; GQueue *queue; };
G_DEFINE_TYPE(VdlQueueManager, vdl_queue_manager, G_TYPE_OBJECT)
static void vdl_queue_manager_finalize(GObject *o) {
    VdlQueueManager *s = VDL_QUEUE_MANAGER(o);
    g_queue_free(s->queue);
    G_OBJECT_CLASS(vdl_queue_manager_parent_class)->finalize(o);
}
static void vdl_queue_manager_class_init(VdlQueueManagerClass *k) {
    G_OBJECT_CLASS(k)->finalize = vdl_queue_manager_finalize;
}
static void vdl_queue_manager_init(VdlQueueManager *s) { s->queue = g_queue_new(); }
VdlQueueManager *vdl_queue_manager_new(VdlHttpEngine *engine) {
    VdlQueueManager *s = g_object_new(VDL_TYPE_QUEUE_MANAGER, NULL);
    s->engine = engine;
    return s;
}
void vdl_queue_manager_add(VdlQueueManager *self, VdlDownloadItem *item) {
    g_queue_push_tail(self->queue, item);
    vdl_logger_info("Download queued: %s", vdl_download_item_get_filename(item));
    vdl_http_engine_start_download(self->engine, item);
}
