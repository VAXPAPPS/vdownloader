/*
 * VXAPDownloader - BitTorrent Engine (C API Header)
 * C wrapper around libtorrent-rasterbar for use in GTK4/C application.
 */

#ifndef VDL_TORRENT_ENGINE_H
#define VDL_TORRENT_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle */
typedef struct VdlTorrentEngine VdlTorrentEngine;

/* Torrent status */
typedef enum {
    VDL_TORRENT_CHECKING,
    VDL_TORRENT_DOWNLOADING_META,
    VDL_TORRENT_DOWNLOADING,
    VDL_TORRENT_FINISHED,
    VDL_TORRENT_SEEDING,
    VDL_TORRENT_PAUSED,
    VDL_TORRENT_ERROR
} VdlTorrentStatus;

/* Torrent info returned by the engine */
typedef struct {
    char     name[512];
    char     save_path[1024];
    char     info_hash[64];
    int64_t  total_size;
    int64_t  downloaded;
    int64_t  uploaded;
    double   download_rate;
    double   upload_rate;
    double   progress;       /* 0.0 - 1.0 */
    int      num_peers;
    int      num_seeds;
    int      eta_seconds;
    VdlTorrentStatus status;
} VdlTorrentInfo;

/* Lifecycle */
VdlTorrentEngine *vdl_torrent_engine_new         (const char *save_path);
void              vdl_torrent_engine_destroy      (VdlTorrentEngine *engine);

/* Add torrents */
int  vdl_torrent_engine_add_torrent_file   (VdlTorrentEngine *engine,
                                             const char *torrent_path,
                                             const char *save_path);
int  vdl_torrent_engine_add_magnet         (VdlTorrentEngine *engine,
                                             const char *magnet_uri,
                                             const char *save_path);

/* Control */
void vdl_torrent_engine_pause              (VdlTorrentEngine *engine, int index);
void vdl_torrent_engine_resume             (VdlTorrentEngine *engine, int index);
void vdl_torrent_engine_remove             (VdlTorrentEngine *engine, int index,
                                             bool delete_files);

/* Query */
int  vdl_torrent_engine_count              (VdlTorrentEngine *engine);
bool vdl_torrent_engine_get_info           (VdlTorrentEngine *engine, int index,
                                             VdlTorrentInfo *info);

/* Session stats */
double vdl_torrent_engine_total_download_rate (VdlTorrentEngine *engine);
double vdl_torrent_engine_total_upload_rate   (VdlTorrentEngine *engine);

#ifdef __cplusplus
}
#endif

#endif /* VDL_TORRENT_ENGINE_H */
