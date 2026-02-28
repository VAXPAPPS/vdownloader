#include "vdl-speed-limiter.h"
struct _VdlSpeedLimiter { GObject parent; int64_t limit; };
G_DEFINE_TYPE(VdlSpeedLimiter, vdl_speed_limiter, G_TYPE_OBJECT)
static void vdl_speed_limiter_class_init(VdlSpeedLimiterClass *k) {}
static void vdl_speed_limiter_init(VdlSpeedLimiter *s) { s->limit = 0; }
VdlSpeedLimiter *vdl_speed_limiter_new(void) { return g_object_new(VDL_TYPE_SPEED_LIMITER, NULL); }
void vdl_speed_limiter_set_limit(VdlSpeedLimiter *self, int64_t bps) { self->limit = bps; }
