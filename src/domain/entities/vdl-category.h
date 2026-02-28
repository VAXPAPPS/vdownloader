/* VXAPDownloader - Category Entity */
#ifndef VDL_CATEGORY_H
#define VDL_CATEGORY_H
#include <glib-object.h>
#include "../../core/vdl-enums.h"
G_BEGIN_DECLS
#define VDL_TYPE_CATEGORY (vdl_category_get_type())
G_DECLARE_FINAL_TYPE (VdlCategory, vdl_category, VDL, CATEGORY, GObject)
VdlCategory *vdl_category_new (const char *name, const char *icon_name, const char *save_path);
const char *vdl_category_get_name (VdlCategory *self);
const char *vdl_category_get_icon_name (VdlCategory *self);
const char *vdl_category_get_save_path (VdlCategory *self);
G_END_DECLS
#endif
