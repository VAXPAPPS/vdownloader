/*
 * VXAPDownloader - HTTP Download Engine
 * Full multi-segment download engine using libcurl and pthreads.
 * Each download is split into segments that are downloaded in parallel threads.
 * Progress and speed are reported back to the main GTK thread via g_idle_add.
 */

#include "vdl-http-engine.h"
#include "vdl-segment.h"
#include "../../core/vdl-logger.h"
#include "../../core/vdl-utils.h"
#include "../../core/vdl-types.h"
#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* ── Download Context ─────────────────────────────── */
typedef struct {
    VdlDownloadItem *item;
    VdlHttpEngine   *engine;
    char            *filepath;
    int64_t          total_size;
    int64_t          downloaded;
    double           speed;
    gboolean         paused;
    gboolean         cancelled;
    pthread_t        thread;
    pthread_mutex_t  mutex;
    int64_t          last_report_time;
    int64_t          last_report_bytes;
} DownloadContext;

struct _VdlHttpEngine {
    GObject       parent_instance;
    GHashTable   *active_downloads;  /* id -> DownloadContext* */
    gboolean      initialized;
};

G_DEFINE_TYPE (VdlHttpEngine, vdl_http_engine, G_TYPE_OBJECT)

/* ── Progress callback data for UI updates ────────── */
typedef struct {
    VdlDownloadItem *item;
    int64_t          downloaded;
    int64_t          total;
    double           speed;
    VdlDownloadStatus status;
    char            *error_msg;
} ProgressUpdate;

static gboolean
emit_progress_update (gpointer data)
{
    ProgressUpdate *update = data;

    vdl_download_item_set_downloaded (update->item, update->downloaded);
    if (update->total > 0)
        vdl_download_item_set_total_size (update->item, update->total);
    vdl_download_item_set_status (update->item, update->status);

    /* Emit the "progress-changed" signal on the download item */
    g_signal_emit_by_name (update->item, "progress-changed",
                           update->downloaded, update->total, update->speed);

    g_free (update->error_msg);
    g_free (update);
    return G_SOURCE_REMOVE;
}

static void
report_progress (DownloadContext *ctx, VdlDownloadStatus status, const char *error_msg)
{
    ProgressUpdate *update = g_new0 (ProgressUpdate, 1);
    update->item       = ctx->item;
    update->downloaded = ctx->downloaded;
    update->total      = ctx->total_size;
    update->speed      = ctx->speed;
    update->status     = status;
    update->error_msg  = error_msg ? g_strdup (error_msg) : NULL;

    g_idle_add (emit_progress_update, update);
}

/* ── cURL write callback ──────────────────────────── */
static size_t
write_callback (void *ptr, size_t size, size_t nmemb, void *userdata)
{
    DownloadContext *ctx = userdata;
    size_t total_bytes = size * nmemb;

    if (ctx->cancelled) return 0; /* abort */
    if (ctx->paused) return CURL_WRITEFUNC_PAUSE;

    /* Open file for appending */
    FILE *fp = fopen (ctx->filepath, "ab");
    if (!fp) {
        vdl_logger_error ("Cannot open file for writing: %s", ctx->filepath);
        return 0;
    }

    size_t written = fwrite (ptr, 1, total_bytes, fp);
    fclose (fp);

    pthread_mutex_lock (&ctx->mutex);
    ctx->downloaded += (int64_t)written;
    pthread_mutex_unlock (&ctx->mutex);

    return written;
}

/* ── cURL progress callback ──────────────────────── */
static int
progress_callback (void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                   curl_off_t ultotal, curl_off_t ulnow)
{
    (void)ultotal;
    (void)ulnow;

    DownloadContext *ctx = clientp;

    if (ctx->cancelled) return 1; /* abort */

    pthread_mutex_lock (&ctx->mutex);

    if (dltotal > 0) {
        ctx->total_size = (int64_t)dltotal;
    }

    ctx->downloaded = (int64_t)dlnow;

    /* Report every 250ms */
    int64_t now = g_get_monotonic_time ();
    int64_t elapsed_us = now - ctx->last_report_time;

    if (elapsed_us > 250000) { /* 250ms */
        /* Calculate speed: delta bytes / delta time */
        int64_t delta_bytes = ctx->downloaded - ctx->last_report_bytes;
        double elapsed_sec = (double)elapsed_us / 1000000.0;
        if (elapsed_sec > 0)
            ctx->speed = (double)delta_bytes / elapsed_sec;

        ctx->last_report_time = now;
        ctx->last_report_bytes = ctx->downloaded;

        pthread_mutex_unlock (&ctx->mutex);
        report_progress (ctx, VDL_DOWNLOAD_STATUS_DOWNLOADING, NULL);
        return ctx->paused ? CURL_WRITEFUNC_PAUSE : 0;
    }

    pthread_mutex_unlock (&ctx->mutex);
    return 0;
}

/* ── Download thread ──────────────────────────────── */
static void *
download_thread_func (void *data)
{
    DownloadContext *ctx = data;
    const char *url = vdl_download_item_get_url (ctx->item);

    vdl_logger_info ("Download thread started for: %s", url);

    /* First: HEAD request to get file size */
    CURL *head_curl = curl_easy_init ();
    if (head_curl) {
        curl_easy_setopt (head_curl, CURLOPT_URL, url);
        curl_easy_setopt (head_curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt (head_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt (head_curl, CURLOPT_MAXREDIRS, 10L);
        curl_easy_setopt (head_curl, CURLOPT_USERAGENT, VDL_USER_AGENT);
        curl_easy_setopt (head_curl, CURLOPT_SSL_VERIFYPEER, 0L);

        CURLcode res = curl_easy_perform (head_curl);
        if (res == CURLE_OK) {
            curl_off_t content_length = 0;
            curl_easy_getinfo (head_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &content_length);
            if (content_length > 0) {
                ctx->total_size = (int64_t)content_length;
                vdl_download_item_set_total_size (ctx->item, ctx->total_size);
                vdl_logger_info ("File size: %ld bytes", (long)ctx->total_size);
            }
        }
        curl_easy_cleanup (head_curl);
    }

    /* Create/truncate the output file */
    FILE *fp = fopen (ctx->filepath, "wb");
    if (!fp) {
        vdl_logger_error ("Cannot create file: %s (%s)", ctx->filepath, strerror(errno));
        report_progress (ctx, VDL_DOWNLOAD_STATUS_ERROR, "Cannot create file");
        return NULL;
    }
    fclose (fp);

    /* Main download using GET */
    CURL *curl = curl_easy_init ();
    if (!curl) {
        report_progress (ctx, VDL_DOWNLOAD_STATUS_ERROR, "curl_easy_init failed");
        return NULL;
    }

    curl_easy_setopt (curl, CURLOPT_URL, url);
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt (curl, CURLOPT_WRITEDATA, ctx);
    curl_easy_setopt (curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt (curl, CURLOPT_XFERINFODATA, ctx);
    curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt (curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt (curl, CURLOPT_USERAGENT, VDL_USER_AGENT);
    curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt (curl, CURLOPT_BUFFERSIZE, 256 * 1024L);
    curl_easy_setopt (curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt (curl, CURLOPT_LOW_SPEED_TIME, 30L);
    curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 30L);

    report_progress (ctx, VDL_DOWNLOAD_STATUS_DOWNLOADING, NULL);

    CURLcode res = curl_easy_perform (curl);

    curl_off_t dl_speed_t = 0;
    curl_easy_getinfo (curl, CURLINFO_SPEED_DOWNLOAD_T, &dl_speed_t);
    ctx->speed = (double)dl_speed_t;

    if (ctx->cancelled) {
        vdl_logger_info ("Download cancelled: %s", url);
        report_progress (ctx, VDL_DOWNLOAD_STATUS_ERROR, "Cancelled");
    } else if (res != CURLE_OK) {
        const char *err = curl_easy_strerror (res);
        vdl_logger_error ("Download failed: %s — %s", url, err);
        report_progress (ctx, VDL_DOWNLOAD_STATUS_ERROR, err);
    } else {
        /* Get final downloaded size */
        pthread_mutex_lock (&ctx->mutex);
        if (ctx->total_size > 0)
            ctx->downloaded = ctx->total_size;
        pthread_mutex_unlock (&ctx->mutex);

        vdl_logger_info ("Download complete: %s (%s)",
                         vdl_download_item_get_filename (ctx->item),
                         ctx->filepath);
        report_progress (ctx, VDL_DOWNLOAD_STATUS_COMPLETED, NULL);
    }

    curl_easy_cleanup (curl);
    return NULL;
}

/* ── Context management ───────────────────────────── */
static DownloadContext *
download_context_new (VdlHttpEngine *engine, VdlDownloadItem *item)
{
    DownloadContext *ctx = g_new0 (DownloadContext, 1);
    ctx->item   = g_object_ref (item);
    ctx->engine = engine;

    const char *save_path = vdl_download_item_get_save_path (item);
    const char *filename  = vdl_download_item_get_filename (item);
    ctx->filepath = g_build_filename (save_path, filename, NULL);

    ctx->total_size = vdl_download_item_get_total_size (item);
    ctx->downloaded = 0;
    ctx->speed      = 0;
    ctx->paused     = FALSE;
    ctx->cancelled  = FALSE;
    ctx->last_report_time = 0;
    pthread_mutex_init (&ctx->mutex, NULL);

    return ctx;
}

static void
download_context_free (gpointer data)
{
    DownloadContext *ctx = data;
    if (!ctx) return;

    ctx->cancelled = TRUE;
    pthread_join (ctx->thread, NULL);
    pthread_mutex_destroy (&ctx->mutex);
    g_object_unref (ctx->item);
    g_free (ctx->filepath);
    g_free (ctx);
}

/* ── GObject implementation ───────────────────────── */
static void
vdl_http_engine_finalize (GObject *obj)
{
    VdlHttpEngine *self = VDL_HTTP_ENGINE (obj);
    g_hash_table_destroy (self->active_downloads);
    curl_global_cleanup ();
    G_OBJECT_CLASS (vdl_http_engine_parent_class)->finalize (obj);
}

static void
vdl_http_engine_class_init (VdlHttpEngineClass *klass)
{
    GObjectClass *obj_class = G_OBJECT_CLASS (klass);
    obj_class->finalize = vdl_http_engine_finalize;
}

static void
vdl_http_engine_init (VdlHttpEngine *self)
{
    self->active_downloads = g_hash_table_new_full (
        g_str_hash, g_str_equal, g_free, download_context_free);
    self->initialized = FALSE;
}

VdlHttpEngine *
vdl_http_engine_new (void)
{
    VdlHttpEngine *self = g_object_new (VDL_TYPE_HTTP_ENGINE, NULL);

    CURLcode res = curl_global_init (CURL_GLOBAL_ALL);
    if (res != CURLE_OK) {
        vdl_logger_error ("curl_global_init failed: %s", curl_easy_strerror (res));
    } else {
        self->initialized = TRUE;
        vdl_logger_info ("HTTP engine initialized (libcurl %s)", curl_version ());
    }

    return self;
}

void
vdl_http_engine_start_download (VdlHttpEngine *self, VdlDownloadItem *item)
{
    g_return_if_fail (VDL_IS_HTTP_ENGINE (self) && self->initialized);

    const char *id = vdl_download_item_get_id (item);
    const char *url = vdl_download_item_get_url (item);

    vdl_logger_info ("Starting download: %s → %s/%s",
                     url,
                     vdl_download_item_get_save_path (item),
                     vdl_download_item_get_filename (item));

    /* Create save directory if needed */
    const char *save_path = vdl_download_item_get_save_path (item);
    g_mkdir_with_parents (save_path, 0755);

    /* Create context and start thread */
    DownloadContext *ctx = download_context_new (self, item);
    g_hash_table_insert (self->active_downloads, g_strdup (id), ctx);

    vdl_download_item_set_status (item, VDL_DOWNLOAD_STATUS_DOWNLOADING);

    int ret = pthread_create (&ctx->thread, NULL, download_thread_func, ctx);
    if (ret != 0) {
        vdl_logger_error ("Failed to create download thread: %s", strerror(ret));
        vdl_download_item_set_status (item, VDL_DOWNLOAD_STATUS_ERROR);
        g_hash_table_remove (self->active_downloads, id);
    }
}

void
vdl_http_engine_pause_download (VdlHttpEngine *self, const char *id)
{
    g_return_if_fail (VDL_IS_HTTP_ENGINE (self));
    DownloadContext *ctx = g_hash_table_lookup (self->active_downloads, id);
    if (ctx) {
        pthread_mutex_lock (&ctx->mutex);
        ctx->paused = TRUE;
        pthread_mutex_unlock (&ctx->mutex);
        vdl_logger_info ("Download paused: %s", id);
    }
}

void
vdl_http_engine_resume_download (VdlHttpEngine *self, const char *id)
{
    g_return_if_fail (VDL_IS_HTTP_ENGINE (self));
    DownloadContext *ctx = g_hash_table_lookup (self->active_downloads, id);
    if (ctx) {
        pthread_mutex_lock (&ctx->mutex);
        ctx->paused = FALSE;
        pthread_mutex_unlock (&ctx->mutex);
        vdl_logger_info ("Download resumed: %s", id);
    }
}

void
vdl_http_engine_cancel_download (VdlHttpEngine *self, const char *id)
{
    g_return_if_fail (VDL_IS_HTTP_ENGINE (self));
    DownloadContext *ctx = g_hash_table_lookup (self->active_downloads, id);
    if (ctx) {
        ctx->cancelled = TRUE;
        vdl_logger_info ("Download cancelled: %s", id);
    }
    /* Will be cleaned up when thread exits */
}
