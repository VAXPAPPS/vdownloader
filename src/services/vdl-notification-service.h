/* VXAPDownloader - Notification Service (stub) */
#ifndef VDL_NOTIFICATION_SERVICE_H
#define VDL_NOTIFICATION_SERVICE_H
#include <glib-object.h>
G_BEGIN_DECLS
#define VDL_TYPE_NOTIFICATION_SERVICE (vdl_notification_service_get_type())
G_DECLARE_FINAL_TYPE(VdlNotificationService,vdl_notification_service,VDL,NOTIFICATION_SERVICE,GObject)
VdlNotificationService *vdl_notification_service_new(void);
void vdl_notification_service_send(VdlNotificationService *self, const char *title, const char *body);
G_END_DECLS
#endif
