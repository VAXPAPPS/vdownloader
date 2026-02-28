/*
 * VXAPDownloader - Download Item Entity
 */

#include "vdl-download-item.h"
#include "../../core/vdl-utils.h"

struct _VdlDownloadItem {
    GObject           parent_instance;

    char             *id;
    char             *url;
    char             *filename;
    char             *save_path;
    int64_t           total_size;
    int64_t           downloaded;
    VdlDownloadStatus status;
    VdlDownloadType   type;
    int               segments;
    int64_t           created_at;
};

G_DEFINE_TYPE (VdlDownloadItem, vdl_download_item, G_TYPE_OBJECT)

/* Signal IDs */
enum {
    SIGNAL_PROGRESS_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0 };

static void
vdl_download_item_finalize (GObject *obj)
{
    VdlDownloadItem *self = VDL_DOWNLOAD_ITEM (obj);
    g_free (self->id);
    g_free (self->url);
    g_free (self->filename);
    g_free (self->save_path);
    G_OBJECT_CLASS (vdl_download_item_parent_class)->finalize (obj);
}

static void
vdl_download_item_class_init (VdlDownloadItemClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS (klass);
    obj_class->finalize = vdl_download_item_finalize;

    /**
     * VdlDownloadItem::progress-changed:
     * @item: the download item
     * @downloaded: bytes downloaded so far
     * @total: total bytes (0 if unknown)
     * @speed: download speed in bytes/sec
     *
     * Emitted when download progress updates.
     */
    signals[SIGNAL_PROGRESS_CHANGED] = g_signal_new (
        "progress-changed",
        VDL_TYPE_DOWNLOAD_ITEM,
        G_SIGNAL_RUN_LAST,
        0,
        NULL, NULL,
        NULL,
        G_TYPE_NONE,
        3,
        G_TYPE_INT64,   /* downloaded */
        G_TYPE_INT64,   /* total */
        G_TYPE_DOUBLE   /* speed */
    );
}

static void
vdl_download_item_init (VdlDownloadItem *self)
{
    self->status = VDL_DOWNLOAD_STATUS_QUEUED;
    self->segments = VDL_DEFAULT_SEGMENTS;
    self->created_at = g_get_real_time () / G_USEC_PER_SEC;
}

VdlDownloadItem *
vdl_download_item_new (const char *url, const char *save_path)
{
    VdlDownloadItem *self = g_object_new (VDL_TYPE_DOWNLOAD_ITEM, NULL);
    self->id = vdl_utils_generate_id ();
    self->url = g_strdup (url);
    self->filename = vdl_utils_filename_from_url (url);
    self->save_path = g_strdup (save_path);
    self->total_size = 0;
    self->downloaded = 0;

    /* Detect type */
    if (vdl_utils_is_magnet_link (url))
        self->type = VDL_DOWNLOAD_TYPE_MAGNET;
    else if (g_str_has_prefix (url, "https://"))
        self->type = VDL_DOWNLOAD_TYPE_HTTPS;
    else if (g_str_has_prefix (url, "ftp://"))
        self->type = VDL_DOWNLOAD_TYPE_FTP;
    else
        self->type = VDL_DOWNLOAD_TYPE_HTTP;

    return self;
}

const char *vdl_download_item_get_id (VdlDownloadItem *self) { return self->id; }
const char *vdl_download_item_get_url (VdlDownloadItem *self) { return self->url; }
const char *vdl_download_item_get_filename (VdlDownloadItem *self) { return self->filename; }
void vdl_download_item_set_filename (VdlDownloadItem *self, const char *name) {
    g_free (self->filename);
    self->filename = g_strdup (name);
}
const char *vdl_download_item_get_save_path (VdlDownloadItem *self) { return self->save_path; }
int64_t vdl_download_item_get_total_size (VdlDownloadItem *self) { return self->total_size; }
void vdl_download_item_set_total_size (VdlDownloadItem *self, int64_t size) { self->total_size = size; }
int64_t vdl_download_item_get_downloaded (VdlDownloadItem *self) { return self->downloaded; }
void vdl_download_item_set_downloaded (VdlDownloadItem *self, int64_t bytes) { self->downloaded = bytes; }
VdlDownloadStatus vdl_download_item_get_status (VdlDownloadItem *self) { return self->status; }
void vdl_download_item_set_status (VdlDownloadItem *self, VdlDownloadStatus s) { self->status = s; }
VdlDownloadType vdl_download_item_get_type_ (VdlDownloadItem *self) { return self->type; }
int vdl_download_item_get_segments (VdlDownloadItem *self) { return self->segments; }
void vdl_download_item_set_segments (VdlDownloadItem *self, int segments) { self->segments = CLAMP(segments, 1, VDL_MAX_SEGMENTS); }

double
vdl_download_item_get_progress (VdlDownloadItem *self)
{
    if (self->total_size <= 0) return 0.0;
    return (double)self->downloaded / (double)self->total_size;
}
