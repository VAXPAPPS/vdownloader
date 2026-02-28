#include "vdl-clipboard-monitor.h"
struct _VdlClipboardMonitor { GObject parent; };
G_DEFINE_TYPE(VdlClipboardMonitor,vdl_clipboard_monitor,G_TYPE_OBJECT)
static void vdl_clipboard_monitor_class_init(VdlClipboardMonitorClass *k) {}
static void vdl_clipboard_monitor_init(VdlClipboardMonitor *s) {}
VdlClipboardMonitor *vdl_clipboard_monitor_new(void) { return g_object_new(VDL_TYPE_CLIPBOARD_MONITOR,NULL); }
