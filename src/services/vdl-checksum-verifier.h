/* VXAPDownloader - Checksum Verifier (stub) */
#ifndef VDL_CHECKSUM_VERIFIER_H
#define VDL_CHECKSUM_VERIFIER_H
#include <glib-object.h>
G_BEGIN_DECLS
#define VDL_TYPE_CHECKSUM_VERIFIER (vdl_checksum_verifier_get_type())
G_DECLARE_FINAL_TYPE(VdlChecksumVerifier,vdl_checksum_verifier,VDL,CHECKSUM_VERIFIER,GObject)
VdlChecksumVerifier *vdl_checksum_verifier_new(void);
G_END_DECLS
#endif
