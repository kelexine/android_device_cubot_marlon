#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/cubot/marlon',
    'hardware/mediatek',
]

blob_fixups: blob_fixups_user_type = {
    # Audio HAL and effect shims
    ('vendor/lib/hw/audio.primary.mt6765.so',
     'vendor/lib64/hw/audio.primary.mt6765.so',
     'vendor/lib/libaudiofoundation.so',
     'vendor/lib64/libaudiofoundation.so',
     'vendor/lib/libnbaio_mono.so',
     'vendor/lib64/libnbaio_mono.so'): blob_fixup()
        .add_needed('libshim_audio.so'),

    # Bluetooth HCI Gabeldorsche status event compatibility
    'vendor/lib64/hw/android.hardware.bluetooth@1.0-impl-mediatek.so': blob_fixup()
        .add_needed('libshim_bt.so'),

    # Libutils refcount and incStrong backwards compatibility
    ('vendor/lib64/hw/vendor.mediatek.hardware.pq@2.6-impl.so',
     'vendor/lib64/hw/android.hardware.thermal@2.0-impl.so'): blob_fixup()
        .replace_needed('libutils.so', 'libutils-v32.so'),

    'vendor/lib64/libmtkcam_stdutils.so': blob_fixup()
        .replace_needed('libutils.so', 'libutils-v30.so'),

    # RIL fusion: detach obsolete libmtkutils to prevent String16 .rodata collisions
    'vendor/lib64/librilfusion.so': blob_fixup()
        .replace_needed('libmtkutils.so', 'libutils.so'),

    # Base shims for nvram, sysenv, applypatch, nfc, health, RIL, and PQ
    ('vendor/lib/libnvram.so',
     'vendor/lib/libsysenv.so',
     'vendor/lib64/libnvram.so',
     'vendor/lib64/libsysenv.so',
     'vendor/bin/applypatch',
     'vendor/bin/nfcstackp-vendor',
     'vendor/lib64/libmtk-ril.so',
     'vendor/bin/hw/vendor.mediatek.hardware.pq@2.2-service',
     'vendor/lib/hw/android.hardware.health@2.0-impl-2.1.so',
     'vendor/lib64/hw/android.hardware.health@2.0-impl-2.1.so',
     'vendor/lib/hw/vendor.mediatek.hardware.pq@2.6-impl.so',
     'vendor/lib64/hw/vendor.mediatek.hardware.pq@2.6-impl.so'): blob_fixup()
        .add_needed('libshim_base.so'),

    # Widevine DRM BoringSSL CBS_init and wpa_supplicant sk_dup
    ('vendor/bin/hw/wpa_supplicant',
     'vendor/lib/libwvhidl.so',
     'vendor/lib64/libwvhidl.so',
     'vendor/lib/mediadrm/libwvdrmengine.so',
     'vendor/lib64/mediadrm/libwvdrmengine.so'): blob_fixup()
        .add_needed('libshim_crypto.so'),

    # AEE AEDV, libaedv, and UDF unwindstack shims
    ('vendor/bin/aee_aedv',
     'vendor/bin/aee_aedv64',
     'vendor/lib/libaedv.so',
     'vendor/lib64/libaedv.so',
     'vendor/lib/libudf.so',
     'vendor/lib64/libudf.so'): blob_fixup()
        .add_needed('libshim_unwindstack.so'),

    # Telephony and RIL 32-bit daemons C++ ABI shims
    ('vendor/bin/bip',
     'vendor/bin/volte_imcb',
     'vendor/bin/volte_imsm_93',
     'vendor/bin/volte_md_status',
     'vendor/bin/volte_stack',
     'vendor/bin/volte_ua',
     'vendor/bin/wfca'): blob_fixup()
        .add_needed('libshim_cxx.so'),

    # Camera PropertyMap and SetTaskProfiles shim
    ('vendor/bin/hw/camerahalserver',
     'vendor/lib/libmtkcam_stdutils.so',
     'vendor/lib64/libmtkcam_stdutils.so'): blob_fixup()
        .add_needed('libshim_camera.so'),

    # VNDK Binder and service manager shims
    ('vendor/bin/vndservicemanager',
     'vendor/bin/STFlashTool'): blob_fixup()
        .add_needed('libshim_binder.so'),

    # GraphicBufferMapper lock/unlock OMX shims
    ('vendor/lib/libMtkOmxVdecEx.so',
     'vendor/lib/libstagefright_softomx.so'): blob_fixup()
        .add_needed('libshim_graphics.so'),

    # Factory AIDL Lights HAL stub
    'vendor/bin/factory': blob_fixup()
        .add_needed('libshim_lights.so'),

    # Face unlock TEE client: remove unused libstdc++.so DT_NEEDED
    ('vendor/lib/libcv_faceid.so',
     'vendor/lib64/libcv_faceid.so'): blob_fixup()
        .remove_needed('libstdc++.so'),

    # MTK RIL CallStack print shim
    ('vendor/lib/libmtk-ril.so',
     'vendor/lib64/libmtk-ril.so'): blob_fixup()
        .add_needed('libshim_ril.so'),
}

module = ExtractUtilsModule(
    'marlon',
    'cubot',
    blob_fixups=blob_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
