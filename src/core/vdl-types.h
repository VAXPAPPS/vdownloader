/*
 * VXAPDownloader - Professional Download Manager for VAXP-OS
 * Copyright (C) 2026 VAXP Organization
 */

#ifndef VDL_TYPES_H
#define VDL_TYPES_H

#include <glib.h>
#include <stdint.h>

/* Forward declarations */
typedef struct _VdlApplication     VdlApplication;
typedef struct _VdlMainWindow      VdlMainWindow;
typedef struct _VdlDownloadItem    VdlDownloadItem;
typedef struct _VdlCategory        VdlCategory;
typedef struct _VdlSidebar         VdlSidebar;
typedef struct _VdlHeaderBar       VdlHeaderBar;
typedef struct _VdlDownloadRow     VdlDownloadRow;
typedef struct _VdlProgressRing    VdlProgressRing;
typedef struct _VdlSpeedGraph      VdlSpeedGraph;
typedef struct _VdlHttpEngine      VdlHttpEngine;
typedef struct _VdlSegment         VdlSegment;
typedef struct _VdlChunkManager    VdlChunkManager;
typedef struct _VdlSpeedLimiter    VdlSpeedLimiter;
typedef struct _VdlQueueManager    VdlQueueManager;
typedef struct _VdlScheduler       VdlScheduler;
typedef struct _VdlSqliteRepo      VdlSqliteRepo;
typedef struct _VdlConfig          VdlConfig;

/* Transfer stats */
typedef struct {
    int64_t  bytes_downloaded;
    int64_t  bytes_uploaded;
    int64_t  total_size;
    double   download_speed;
    double   upload_speed;
    double   progress;
    int      eta_seconds;
    int      active_connections;
} VdlTransferStats;

/* Speed sample for graphs */
typedef struct {
    double   speed;
    int64_t  timestamp;
} VdlSpeedSample;

/* App-wide constants */
#define VDL_APP_ID          "org.vaxp.downloader"
#define VDL_APP_NAME        "VXAP Downloader"
#define VDL_APP_VERSION     "1.0.0"
#define VDL_MAX_SEGMENTS     32
#define VDL_DEFAULT_SEGMENTS 8
#define VDL_MAX_CONCURRENT   8
#define VDL_SPEED_HISTORY    120  /* seconds of speed graph history */
#define VDL_USER_AGENT       "VXAPDownloader/1.0"

#endif /* VDL_TYPES_H */
