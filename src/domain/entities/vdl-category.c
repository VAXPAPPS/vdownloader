/*
 * VXAPDownloader - Category Entity
 */
#include "vdl-category.h"

struct _VdlCategory {
    GObject parent_instance;
    char *name;
    char *icon_name;
    char *save_path;
    VdlCategoryType type;
};

G_DEFINE_TYPE (VdlCategory, vdl_category, G_TYPE_OBJECT)

static void vdl_category_finalize (GObject *obj) {
    VdlCategory *self = VDL_CATEGORY (obj);
    g_free (self->name);
    g_free (self->icon_name);
    g_free (self->save_path);
    G_OBJECT_CLASS (vdl_category_parent_class)->finalize (obj);
}

static void vdl_category_class_init (VdlCategoryClass *klass) {
    G_OBJECT_CLASS (klass)->finalize = vdl_category_finalize;
}

static void vdl_category_init (VdlCategory *self) {
    self->type = VDL_CATEGORY_GENERAL;
}

VdlCategory *vdl_category_new (const char *name, const char *icon_name, const char *save_path) {
    VdlCategory *self = g_object_new (VDL_TYPE_CATEGORY, NULL);
    self->name = g_strdup (name);
    self->icon_name = g_strdup (icon_name);
    self->save_path = g_strdup (save_path);
    return self;
}

const char *vdl_category_get_name (VdlCategory *self) { return self->name; }
const char *vdl_category_get_icon_name (VdlCategory *self) { return self->icon_name; }
const char *vdl_category_get_save_path (VdlCategory *self) { return self->save_path; }
