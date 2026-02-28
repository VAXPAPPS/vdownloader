#include "vdl-checksum-verifier.h"
struct _VdlChecksumVerifier { GObject parent; };
G_DEFINE_TYPE(VdlChecksumVerifier,vdl_checksum_verifier,G_TYPE_OBJECT)
static void vdl_checksum_verifier_class_init(VdlChecksumVerifierClass *k) {}
static void vdl_checksum_verifier_init(VdlChecksumVerifier *s) {}
VdlChecksumVerifier *vdl_checksum_verifier_new(void) {
    return g_object_new(VDL_TYPE_CHECKSUM_VERIFIER, NULL);
}
