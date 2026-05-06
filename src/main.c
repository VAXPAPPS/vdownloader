#include "app/vxap_app.h"

int
main (int argc, char *argv[])
{
    VxapApp *app = vxap_app_new ();
    int status = g_application_run (G_APPLICATION(app), argc, argv);
    g_object_unref (app);
    return status;
}
