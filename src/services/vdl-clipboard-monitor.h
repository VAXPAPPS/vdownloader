/* VXAPDownloader - Clipboard Monitor (stub) */
#ifndef VDL_CLIPBOARD_MONITOR_H
#define VDL_CLIPBOARD_MONITOR_H
#include <glib-object.h>
G_BEGIN_DECLS
#define VDL_TYPE_CLIPBOARD_MONITOR (vdl_clipboard_monitor_get_type())
G_DECLARE_FINAL_TYPE(VdlClipboardMonitor,vdl_clipboard_monitor,VDL,CLIPBOARD_MONITOR,GObject)
VdlClipboardMonitor *vdl_clipboard_monitor_new(void);
G_END_DECLS
#endif
