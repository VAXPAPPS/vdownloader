#pragma once

#include <glib-object.h>
#include "aria2_client.h"

#define VXAP_TYPE_ARIA2_MONITOR (aria2_monitor_get_type())
G_DECLARE_FINAL_TYPE(Aria2Monitor, aria2_monitor, VXAP, ARIA2_MONITOR, GObject)

/* Signals emitted by Aria2Monitor:
 *  "downloads-updated"  : GList *items  (all known downloads)
 *  "stats-updated"      : dl_speed, ul_speed, num_active, num_waiting, num_stopped
 *  "download-complete"  : DownloadItem *item
 */

Aria2Monitor *aria2_monitor_new         (Aria2Client *client);
void          aria2_monitor_start       (Aria2Monitor *self);
void          aria2_monitor_stop        (Aria2Monitor *self);
gboolean      aria2_monitor_is_running  (Aria2Monitor *self);
void          aria2_monitor_set_interval(Aria2Monitor *self, guint ms);
