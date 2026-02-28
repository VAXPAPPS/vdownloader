/* VXAPDownloader - Scheduler (stub) */
#ifndef VDL_SCHEDULER_H
#define VDL_SCHEDULER_H
#include <glib-object.h>
G_BEGIN_DECLS
#define VDL_TYPE_SCHEDULER (vdl_scheduler_get_type())
G_DECLARE_FINAL_TYPE(VdlScheduler, vdl_scheduler, VDL, SCHEDULER, GObject)
VdlScheduler *vdl_scheduler_new(void);
G_END_DECLS
#endif
