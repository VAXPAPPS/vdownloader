#include "vdl-scheduler.h"
struct _VdlScheduler { GObject parent; };
G_DEFINE_TYPE(VdlScheduler, vdl_scheduler, G_TYPE_OBJECT)
static void vdl_scheduler_class_init(VdlSchedulerClass *k) {}
static void vdl_scheduler_init(VdlScheduler *s) {}
VdlScheduler *vdl_scheduler_new(void) { return g_object_new(VDL_TYPE_SCHEDULER, NULL); }
