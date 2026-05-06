#pragma once

#ifdef HAVE_ADWAITA
#include <adwaita.h>
#define VxapApplicationParent AdwApplication
#define VXAP_TYPE_APPLICATION (vxap_app_get_type())
G_DECLARE_FINAL_TYPE(VxapApp, vxap_app, VXAP, APP, AdwApplication)
#else
#include <gtk/gtk.h>
#define VxapApplicationParent GtkApplication
#define VXAP_TYPE_APPLICATION (vxap_app_get_type())
G_DECLARE_FINAL_TYPE(VxapApp, vxap_app, VXAP, APP, GtkApplication)
#endif

VxapApp *vxap_app_new (void);
