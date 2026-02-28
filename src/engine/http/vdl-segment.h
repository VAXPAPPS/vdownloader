/* VXAPDownloader - Segment */
#ifndef VDL_SEGMENT_H
#define VDL_SEGMENT_H
#include <glib.h>
#include <stdint.h>
#include "../../core/vdl-enums.h"
#include "../../core/vdl-types.h"

struct _VdlSegment {
    int index;
    int64_t start_byte;
    int64_t end_byte;
    int64_t downloaded;
    VdlSegmentStatus status;
};

VdlSegment *vdl_segment_new (int index, int64_t start, int64_t end);
void vdl_segment_free (VdlSegment *seg);
#endif
