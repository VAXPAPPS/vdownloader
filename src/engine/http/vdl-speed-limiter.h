/* VXAPDownloader - Speed Limiter (stub) */
#ifndef VDL_SPEED_LIMITER_H
#define VDL_SPEED_LIMITER_H
#include <glib-object.h>
G_BEGIN_DECLS
#define VDL_TYPE_SPEED_LIMITER (vdl_speed_limiter_get_type())
G_DECLARE_FINAL_TYPE(VdlSpeedLimiter, vdl_speed_limiter, VDL, SPEED_LIMITER, GObject)
VdlSpeedLimiter *vdl_speed_limiter_new(void);
void vdl_speed_limiter_set_limit(VdlSpeedLimiter *self, int64_t bytes_per_sec);
G_END_DECLS
#endif
