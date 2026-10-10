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
    # Audio primary TypeConverter shim
    'vendor/lib/hw/audio.primary.mt6765.so': blob_fixup()
        .add_needed('libshim_audio.so'),

    # Libutils refcount and incStrong backwards compatibility
    ('vendor/lib64/hw/vendor.mediatek.hardware.pq@2.6-impl.so',
     'vendor/lib64/hw/android.hardware.thermal@2.0-impl.so'): blob_fixup()
        .replace_needed('libutils.so', 'libutils-v32.so'),

    'vendor/lib64/libmtkcam_stdutils.so': blob_fixup()
        .replace_needed('libutils.so', 'libutils-v30.so'),

    # Base shims for nvram and system env (Basename, Trim)
    ('vendor/lib/libnvram.so',
     'vendor/lib/libsysenv.so',
     'vendor/lib64/libnvram.so',
     'vendor/lib64/libsysenv.so'): blob_fixup()
        .add_needed('libshim_base.so'),

    # Widevine DRM BoringSSL CBS_init
    ('vendor/lib/libwvhidl.so',
     'vendor/lib/mediadrm/libwvdrmengine.so'): blob_fixup()
        .add_needed('libshim_base.so'),

    # AEE AEDV unwindstack shims
    ('vendor/bin/aee_aedv',
     'vendor/bin/aee_aedv64'): blob_fixup()
        .add_needed('libshim_base.so'),

    # Telephony and RIL 32-bit daemons C++ ABI shims
    ('vendor/bin/bip',
     'vendor/bin/volte_imcb',
     'vendor/bin/volte_stack',
     'vendor/bin/volte_ua',
     'vendor/bin/volte_imsm_93',
     'vendor/bin/wfca'): blob_fixup()
        .add_needed('libshim_base.so'),
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
