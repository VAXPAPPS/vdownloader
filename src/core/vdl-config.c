/*
 * VXAPDownloader - Configuration Manager Implementation
 */

#include "vdl-config.h"
#include "vdl-types.h"
#include "vdl-logger.h"
#include <json-glib/json-glib.h>

struct _VdlConfig {
    GObject   parent_instance;

    char     *download_dir;
    int       max_concurrent;
    int       segments;
    int64_t   speed_limit;
    gboolean  clipboard_watch;
    gboolean  notifications;
    gboolean  auto_start;
    char     *config_path;
};

G_DEFINE_TYPE (VdlConfig, vdl_config, G_TYPE_OBJECT)

static void
vdl_config_load (VdlConfig *self)
{
    if (!g_file_test (self->config_path, G_FILE_TEST_EXISTS)) {
        vdl_logger_info ("No config file found, using defaults.");
        return;
    }

    g_autoptr(JsonParser) parser = json_parser_new ();
    g_autoptr(GError) error = NULL;

    if (!json_parser_load_from_file (parser, self->config_path, &error)) {
        vdl_logger_error ("Failed to load config: %s", error->message);
        return;
    }

    JsonNode *root_node = json_parser_get_root (parser);
    JsonObject *obj = json_node_get_object (root_node);

    if (json_object_has_member (obj, "download_dir")) {
        g_free (self->download_dir);
        self->download_dir = g_strdup (json_object_get_string_member (obj, "download_dir"));
    }
    if (json_object_has_member (obj, "max_concurrent"))
        self->max_concurrent = (int) json_object_get_int_member (obj, "max_concurrent");
    if (json_object_has_member (obj, "segments"))
        self->segments = (int) json_object_get_int_member (obj, "segments");
    if (json_object_has_member (obj, "speed_limit"))
        self->speed_limit = json_object_get_int_member (obj, "speed_limit");
    if (json_object_has_member (obj, "clipboard_watch"))
        self->clipboard_watch = json_object_get_boolean_member (obj, "clipboard_watch");

    vdl_logger_info ("Config loaded from %s", self->config_path);
}

static void
vdl_config_finalize (GObject *object)
{
    VdlConfig *self = VDL_CONFIG (object);
    g_free (self->download_dir);
    g_free (self->config_path);
    G_OBJECT_CLASS (vdl_config_parent_class)->finalize (object);
}

static void
vdl_config_class_init (VdlConfigClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS (klass);
    obj_class->finalize = vdl_config_finalize;
}

static void
vdl_config_init (VdlConfig *self)
{
    self->download_dir    = g_build_filename (g_get_user_special_dir (G_USER_DIRECTORY_DOWNLOAD), NULL);
    self->max_concurrent  = VDL_MAX_CONCURRENT;
    self->segments        = VDL_DEFAULT_SEGMENTS;
    self->speed_limit     = 0; /* 0 = unlimited */
    self->clipboard_watch = TRUE;
    self->notifications   = TRUE;
    self->auto_start      = TRUE;
}

VdlConfig *
vdl_config_new (void)
{
    VdlConfig *self = g_object_new (VDL_TYPE_CONFIG, NULL);

    g_autofree char *config_dir = g_build_filename (
        g_get_user_config_dir (), "vxap-downloader", NULL
    );
    g_mkdir_with_parents (config_dir, 0755);
    self->config_path = g_build_filename (config_dir, "config.json", NULL);

    vdl_config_load (self);
    return self;
}

void
vdl_config_save (VdlConfig *self)
{
    g_return_if_fail (VDL_IS_CONFIG (self));

    g_autoptr(JsonBuilder) builder = json_builder_new ();
    json_builder_begin_object (builder);
    json_builder_set_member_name (builder, "download_dir");
    json_builder_add_string_value (builder, self->download_dir);
    json_builder_set_member_name (builder, "max_concurrent");
    json_builder_add_int_value (builder, self->max_concurrent);
    json_builder_set_member_name (builder, "segments");
    json_builder_add_int_value (builder, self->segments);
    json_builder_set_member_name (builder, "speed_limit");
    json_builder_add_int_value (builder, self->speed_limit);
    json_builder_set_member_name (builder, "clipboard_watch");
    json_builder_add_boolean_value (builder, self->clipboard_watch);
    json_builder_end_object (builder);

    g_autoptr(JsonGenerator) gen = json_generator_new ();
    json_generator_set_pretty (gen, TRUE);
    g_autoptr(JsonNode) root = json_builder_get_root (builder);
    json_generator_set_root (gen, root);

    g_autoptr(GError) error = NULL;
    if (!json_generator_to_file (gen, self->config_path, &error)) {
        vdl_logger_error ("Failed to save config: %s", error->message);
    } else {
        vdl_logger_info ("Config saved to %s", self->config_path);
    }
}

const char *vdl_config_get_download_dir (VdlConfig *self) { return self->download_dir; }
void vdl_config_set_download_dir (VdlConfig *self, const char *dir) {
    g_free (self->download_dir);
    self->download_dir = g_strdup (dir);
}
int vdl_config_get_max_concurrent (VdlConfig *self) { return self->max_concurrent; }
void vdl_config_set_max_concurrent (VdlConfig *self, int max) { self->max_concurrent = CLAMP(max, 1, 32); }
int vdl_config_get_segments (VdlConfig *self) { return self->segments; }
void vdl_config_set_segments (VdlConfig *self, int segments) { self->segments = CLAMP(segments, 1, VDL_MAX_SEGMENTS); }
int64_t vdl_config_get_speed_limit (VdlConfig *self) { return self->speed_limit; }
void vdl_config_set_speed_limit (VdlConfig *self, int64_t limit) { self->speed_limit = MAX(limit, 0); }
gboolean vdl_config_get_clipboard_watch (VdlConfig *self) { return self->clipboard_watch; }
void vdl_config_set_clipboard_watch (VdlConfig *self, gboolean watch) { self->clipboard_watch = watch; }
