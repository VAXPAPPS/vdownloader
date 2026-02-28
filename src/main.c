/*
 * VXAPDownloader - Professional Download Manager for VAXP-OS
 * Copyright (C) 2026 VAXP Organization
 */

#include "core/vdl-application.h"

int
main (int argc, char *argv[])
{
    g_autoptr(VdlApplication) app = vdl_application_new ();
    return g_application_run (G_APPLICATION (app), argc, argv);
}
