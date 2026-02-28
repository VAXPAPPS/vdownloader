/*
 * VXAPDownloader - Configuration Manager
 */

#ifndef VDL_CONFIG_H
#define VDL_CONFIG_H

#include <glib-object.h>

G_BEGIN_DECLS

#define VDL_TYPE_CONFIG (vdl_config_get_type())
G_DECLARE_FINAL_TYPE (VdlConfig, vdl_config, VDL, CONFIG, GObject)

VdlConfig   *vdl_config_new                (void);
const char  *vdl_config_get_download_dir   (VdlConfig *self);
void         vdl_config_set_download_dir   (VdlConfig *self, const char *dir);
int          vdl_config_get_max_concurrent (VdlConfig *self);
void         vdl_config_set_max_concurrent (VdlConfig *self, int max);
int          vdl_config_get_segments       (VdlConfig *self);
void         vdl_config_set_segments       (VdlConfig *self, int segments);
int64_t      vdl_config_get_speed_limit    (VdlConfig *self);
void         vdl_config_set_speed_limit    (VdlConfig *self, int64_t limit);
gboolean     vdl_config_get_clipboard_watch(VdlConfig *self);
void         vdl_config_set_clipboard_watch(VdlConfig *self, gboolean watch);
void         vdl_config_save               (VdlConfig *self);

G_END_DECLS

#endif /* VDL_CONFIG_H */
