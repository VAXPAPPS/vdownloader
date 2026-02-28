#include "vdl-notification-service.h"
#include <libnotify/notify.h>
struct _VdlNotificationService { GObject parent; };
G_DEFINE_TYPE(VdlNotificationService,vdl_notification_service,G_TYPE_OBJECT)
static void vdl_notification_service_class_init(VdlNotificationServiceClass *k) {}
static void vdl_notification_service_init(VdlNotificationService *s) {
    notify_init("VXAP Downloader");
}
VdlNotificationService *vdl_notification_service_new(void) {
    return g_object_new(VDL_TYPE_NOTIFICATION_SERVICE, NULL);
}
void vdl_notification_service_send(VdlNotificationService *self, const char *title, const char *body) {
    (void)self;
    NotifyNotification *n = notify_notification_new(title, body, "dialog-information");
    notify_notification_show(n, NULL);
    g_object_unref(n);
}
