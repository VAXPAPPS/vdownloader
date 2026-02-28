/*
 * VXAPDownloader - Professional Download Manager for VAXP-OS
 * Copyright (C) 2026 VAXP Organization
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef VDL_ENUMS_H
#define VDL_ENUMS_H

/* Download status */
typedef enum {
    VDL_DOWNLOAD_STATUS_QUEUED,
    VDL_DOWNLOAD_STATUS_DOWNLOADING,
    VDL_DOWNLOAD_STATUS_PAUSED,
    VDL_DOWNLOAD_STATUS_COMPLETED,
    VDL_DOWNLOAD_STATUS_ERROR,
    VDL_DOWNLOAD_STATUS_MERGING,
    VDL_DOWNLOAD_STATUS_VERIFYING,
    VDL_DOWNLOAD_STATUS_SEEDING,
} VdlDownloadStatus;

/* Download type */
typedef enum {
    VDL_DOWNLOAD_TYPE_HTTP,
    VDL_DOWNLOAD_TYPE_HTTPS,
    VDL_DOWNLOAD_TYPE_FTP,
    VDL_DOWNLOAD_TYPE_TORRENT,
    VDL_DOWNLOAD_TYPE_MAGNET,
} VdlDownloadType;

/* Download priority */
typedef enum {
    VDL_PRIORITY_LOW,
    VDL_PRIORITY_NORMAL,
    VDL_PRIORITY_HIGH,
    VDL_PRIORITY_URGENT,
} VdlPriority;

/* Category type */
typedef enum {
    VDL_CATEGORY_ALL,
    VDL_CATEGORY_GENERAL,
    VDL_CATEGORY_VIDEO,
    VDL_CATEGORY_MUSIC,
    VDL_CATEGORY_DOCUMENTS,
    VDL_CATEGORY_PROGRAMS,
    VDL_CATEGORY_COMPRESSED,
    VDL_CATEGORY_TORRENT,
    VDL_CATEGORY_CUSTOM,
} VdlCategoryType;

/* Sidebar view */
typedef enum {
    VDL_VIEW_DASHBOARD,
    VDL_VIEW_ALL_DOWNLOADS,
    VDL_VIEW_ACTIVE,
    VDL_VIEW_COMPLETED,
    VDL_VIEW_TORRENTS,
    VDL_VIEW_CATEGORIES,
    VDL_VIEW_STATISTICS,
    VDL_VIEW_SETTINGS,
} VdlViewType;

/* Segment status */
typedef enum {
    VDL_SEGMENT_IDLE,
    VDL_SEGMENT_DOWNLOADING,
    VDL_SEGMENT_COMPLETED,
    VDL_SEGMENT_ERROR,
    VDL_SEGMENT_MERGING,
} VdlSegmentStatus;

#endif /* VDL_ENUMS_H */
