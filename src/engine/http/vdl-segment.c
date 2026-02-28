#include "vdl-segment.h"
VdlSegment *vdl_segment_new (int index, int64_t start, int64_t end) {
    VdlSegment *s = g_new0(VdlSegment, 1);
    s->index = index; s->start_byte = start; s->end_byte = end;
    s->downloaded = 0; s->status = VDL_SEGMENT_IDLE;
    return s;
}
void vdl_segment_free (VdlSegment *seg) { g_free(seg); }
